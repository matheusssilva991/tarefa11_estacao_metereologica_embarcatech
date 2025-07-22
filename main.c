#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"
#include <math.h>

#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "lwip/tcp.h"

#include "lib/led/led.h"
#include "lib/button/button.h"
#include "lib/ws2812b/ws2812b.h"
#include "lib/buzzer/buzzer.h"
#include "lib/aht20/aht20.h"
#include "lib/bmp280/bmp280.h"
#include "lib/joystick/joystick.h"

#include "config/wifi_config.h"
#include "public/html_data.h"

#define I2C0_PORT i2c0              // i2c0 pinos 0 e 1
#define I2C0_SDA 0                  // 0
#define I2C0_SCL 1                  // 1
#define I2C1_PORT i2c1              // i2c1 pinos 2 e 3
#define I2C1_SDA 2                  // 2
#define I2C1_SCL 3                  // 3
#define SEA_LEVEL_PRESSURE 101325.0 // 101325.0 // Pressão ao nível do mar em Pa

// Tipos de dados
struct http_state
{
    char response[20000];
    size_t len;
    size_t sent;
};

typedef struct weather_data
{
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    int minTemperature;
    int maxTemperature;
    float offsetTemperature;
} weather_data_t;

// Prototipos
void get_simulated_data(weather_data_t *data);
double calculate_altitude(double pressure);
void check_alerts();
void check_climate_conditions();
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void start_http_server(void);
void gpio_irq_handler(uint gpio, uint32_t events);
bool try_wifi_connect(void);

// Variáveis globais
static weather_data_t weather_data = {0, 0, 0, 0, 10, 70}; // Dados do tempo
static volatile int64_t last_button_a_press_time = 0;      // Tempo do último pressionamento de botão
static volatile int64_t last_button_b_press_time = 0;      // Tempo do último pressionamento de botão B
static volatile int64_t last_button_sw_press_time = 0;     // Tempo do último pressionamento do botão SW
static volatile bool is_alert_active = true;               // Flag para indicar se o alerta está ativo
static volatile bool is_simulated = false;                 // Flag para simulação de dados
static volatile bool wifi_connected = false;
static volatile bool server_started = false;
static uint64_t last_wifi_check = 0;


int main()
{
    stdio_init_all();

    init_btns();
    init_btn(BTN_SW_PIN);
    init_leds_pwm();
    init_joystick();
    ws2812b_init();

    init_buzzer(BUZZER_A_PIN, 4.0f); // Inicializa o buzzer A
    init_buzzer(BUZZER_B_PIN, 4.0f); // Inicializa o buzzer B

    gpio_set_irq_enabled_with_callback(BTN_SW_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BTN_A_PIN, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Inicializa o I2C para o SMP280
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);

    // Inicializa o I2C para o AHT20
    i2c_init(I2C1_PORT, 400 * 1000);
    gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA);
    gpio_pull_up(I2C1_SCL);

    // Inicializa o BMP280
    bmp280_init(I2C0_PORT);
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C0_PORT, &params);

    // Inicializa o AHT20
    aht20_reset(I2C1_PORT);
    aht20_init(I2C1_PORT);

    if (cyw43_arch_init())
    {
        printf("Falha ao inicializar a arquitetura CYW43\n ");
        set_led_red_pwm(); // LED vermelho para falha de inicialização
        sleep_ms(2000);
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    // Loop até conseguir conectar ao Wi-Fi pela primeira vez
    while (!wifi_connected)
    {
        wifi_connected = try_wifi_connect();

        if (!wifi_connected)
        {
            printf("Nova tentativa em 5 segundos...\n");
            sleep_ms(5000);
        }
    }

    // Só inicia o servidor HTTP após conectar ao Wi-Fi
    start_http_server();
    server_started = true;

    // Estruturas para leitura de sensores
    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;
    double altitude;
    uint64_t current_time;

    // Loop principal
    while (true)
    {
        cyw43_arch_poll(); // Mantém o Wi-Fi funcionando

        // Verifica periodicamente o status da conexão Wi-Fi (a cada 10 segundos)
        current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_wifi_check > 10000)
        {
            last_wifi_check = current_time;

            // Verifica o status do link Wi-Fi
            if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
            {
                printf("Conexão Wi-Fi perdida! Tentando reconectar...\n");
                wifi_connected = false;

                // Tenta reconectar
                while (!wifi_connected)
                {
                    wifi_connected = try_wifi_connect();

                    if (!wifi_connected)
                    {
                        printf("Nova tentativa em 5 segundos...\n");
                        sleep_ms(5000);
                    }
                }

                // Reinicia o servidor HTTP se necessário
                if (wifi_connected && !server_started)
                {
                    start_http_server();
                    server_started = true;
                }
            }
        }

        if (is_simulated)
        {
            get_simulated_data(&weather_data);
        }
        else
        {
            // Leitura do BMP280
            bmp280_read_raw(I2C0_PORT, &raw_temp_bmp, &raw_pressure);

            weather_data.temperature = bmp280_convert_temp(raw_temp_bmp, &params);
            weather_data.temperature /= 100.0; // Converte para Celsius

            weather_data.pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
            weather_data.pressure /= 100.0; // Converte para hPa
            weather_data.altitude = calculate_altitude(weather_data.pressure * 100.0); // Converte hPa para Pa

            /* printf("Dados BMP280: Temp=%.2f°C, Press=%.2f hPa, Alt=%.2f m\n",
                   weather_data.temperature, weather_data.pressure, weather_data.altitude); */

            // Leitura do AHT20
            if (aht20_read(I2C1_PORT, &data))
            {
                weather_data.humidity = data.humidity;
                /* printf("Dados AHT20: Temp=%.2f°C, Hum=%.2f%%\n",
                       data.temperature, data.humidity); */
            }
            else
            {
                printf("Erro na leitura do AHT20!\n");
                weather_data.humidity = 0.0; // Valor padrão em caso de erro
            }
        }

        // Verifica os alertas
        check_alerts();

        // Verifica as condições climáticas
        check_climate_conditions();

        sleep_ms(1000);
    }
    cyw43_arch_deinit(); // Esperamos que nunca chegue aqui
}

// Função para calcular a altitude a partir da pressão atmosférica
double calculate_altitude(double pressure)
{
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

// Função para verificar os alertas de temperatura
void check_alerts()
{
    if (is_alert_active)
    {
        if (weather_data.temperature + weather_data.offsetTemperature > weather_data.maxTemperature)
        {
            printf("Alerta: Temperatura acima do limite!\n");
            play_tone(BUZZER_A_PIN, 700); // Toca o buzzer A
            sleep_ms(250);                //
            stop_tone(BUZZER_A_PIN);      // Para o buzzer A
        }
        else if (weather_data.temperature + weather_data.offsetTemperature < weather_data.minTemperature)
        {
            printf("Alerta: Temperatura abaixo do limite!\n");
            play_tone(BUZZER_B_PIN, 400); // Toca o buzzer B
            sleep_ms(250);                //
            stop_tone(BUZZER_B_PIN);      // Para o buzzer B
        }
    }
}

// Função para verificar as condições climáticas
void check_climate_conditions()
{
    bool is_hot = weather_data.temperature > 30;
    bool is_very_hot = weather_data.temperature > 50;
    bool is_cold = weather_data.temperature < 15;
    bool is_very_cold = weather_data.temperature < 5;
    bool is_humid = weather_data.humidity > 80;
    bool is_dry = weather_data.humidity < 20;

    ws2812b_clear(); // Limpa os LEDs

    if (is_hot)
    {
        ws2812b_fill_row(0, 0, 0, 32); // Preenche a primeira linha com azul forte
        ws2812b_fill_row(1, 0, 0, 16); // Preenche a segunda linha com azul fraco
        ws2812b_fill_row(2, 0, 32, 0);
        ws2812b_fill_row(3, 32, 0, 0); // Preenche a primeira linha com vermelho fraco

        if (is_very_hot)
        {
            ws2812b_fill_row(4, 32, 0, 0); // Preenche a última linha com vermelho forte
        }
    }
    else if (is_cold)
    {
        if (is_very_cold)
        {
            ws2812b_fill_row(0, 0, 0, 32); // Preenche a primeira linha com azul forte
        }
        else
        {
            ws2812b_fill_row(0, 0, 0, 32); // Preenche a primeira linha com azul fraco
            ws2812b_fill_row(1, 0, 0, 16); // Preenche a segunda linha com azul fraco
        }
    }
    else
    {
        ws2812b_fill_row(0, 0, 0, 32); // Preenche a primeira linha com azul forte
        ws2812b_fill_row(1, 0, 0, 16); // Preenche a segunda linha com azul fraco
        ws2812b_fill_row(2, 0, 32, 0); // Preenche a terceira linha com verde
    }

    ws2812b_write(); // Atualiza a matriz de LEDs

    if (is_humid)
    {
        set_led_blue_pwm(); // LED ciano para clima úmido
    }
    else if (is_dry)
    {
        set_led_red_pwm(); // LED amarelo para clima seco
    }
    else
    {
        set_led_green_pwm(); // LED branco para clima normal
    }
}

// Função para obter dados simulados do AHT20
void get_simulated_data(weather_data_t *data)
{
    // Simula dados de temperatura e umidade
    data->temperature = get_joystick_y() / 4095.0 * 100.0; // Temperatura entre 0.0 e 60.0 C
    data->humidity = get_joystick_x() / 4095.0 * 100.0;    // Umidade entre 0.0 e 100.0

    //printf("Dados simulados: Temperatura: %.2f C, Umidade: %.2f %%\n", data->temperature, data->humidity);
}

// Função de callback para enviar dados HTTP
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

// Função de recebimento HTTP
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs)
    {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    if (strstr(req, "POST /api/limits"))
    {
        char *body = strstr(req, "\r\n\r\n");
        if (body)
        {
            body += 4;
            int max_val, min_val;
            float offset_val = 0.0f;
            if (sscanf(body, "{\"min\":%d,\"max\":%d,\"offset\":%f", &min_val, &max_val, &offset_val) == 3)
            {
                // **DEBUG: Mostra os limites recebidos**
                printf("Limites recebidos: Max=%d, Min=%d, Offset=%f\n", max_val, min_val, offset_val);
                if (max_val >= 0 && max_val <= 100 && min_val >= -50 && min_val <= 50)
                {
                    weather_data.maxTemperature = max_val;
                    weather_data.minTemperature = min_val;
                    weather_data.offsetTemperature = offset_val; // Atualiza o offset de temperatura
                }
            }
        }

        printf("Novos limites: Max=%d, Min=%d, Offset=%f\n",
               weather_data.maxTemperature,
               weather_data.minTemperature,
               weather_data.offsetTemperature);

        const char *txt = "Limites atualizados";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                           "Access-Control-Allow-Headers: Content-Type\r\n"
                           "Content-Length: %d\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    }
    else if (strstr(req, "GET /api/weather"))
    {
        char json_data[2048];
        snprintf(json_data, sizeof(json_data),
                 "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f,\"altitude\":%.2f,\"minTemperature\":%d,\"maxTemperature\":%d,\"tempOffset\":%.2f}",
                 weather_data.temperature, weather_data.humidity,
                 weather_data.pressure, weather_data.altitude,
                 weather_data.minTemperature, weather_data.maxTemperature, weather_data.offsetTemperature);

        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                           "Access-Control-Allow-Headers: Content-Type\r\n"
                           "Content-Length: %d\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(json_data), json_data);

        //printf("JSON enviado: %s\n", json_data);
    }
    else
    {
        // **HTML principal**
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(html_data), html_data);
    }

    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    pbuf_free(p);
    return ERR_OK;
}

// Função de callback para aceitar conexões TCP
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Função para iniciar o servidor HTTP
static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

// Função de interrupção para os botões
// Reinicia o dispositivo para o modo de boot USB ou alterna o estado do alerta
void gpio_irq_handler(uint gpio, uint32_t events)
{
    int current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_SW_PIN && (current_time - last_button_sw_press_time > 300))
    {
        // Atualiza o tempo do último pressionamento do botão SW
        last_button_sw_press_time = current_time;

        is_simulated = !is_simulated; // Alterna o estado de simulação
    }
    else if (gpio == BTN_A_PIN && (current_time - last_button_a_press_time > 300))
    {
        // Atualiza o tempo do último pressionamento do botão A
        last_button_a_press_time = current_time;

        // Alterna o estado do alerta
        is_alert_active = !is_alert_active;
    }
    else if (gpio == BTN_B_PIN && (current_time - last_button_b_press_time > 300))
    {
        // Atualiza o tempo do último pressionamento do botão B
        last_button_b_press_time = current_time;

        weather_data.minTemperature = 10;  // Reseta o limite mínimo de temperatura
        weather_data.maxTemperature = 70; // Reseta o limite máximo de temperatura
    }
}

// Função para tentar conectar ao Wi-Fi
// Retorna true se a conexão for bem-sucedida, false caso contrário
bool try_wifi_connect()
{
    printf("Tentando conectar ao Wi-Fi '%s'...\n", WIFI_SSID);

    set_led_blue_pwm(); // LED azul para indicar tentativa de conexão

    // Tenta conectar com timeout de 10 segundos
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        set_led_red_pwm(); // LED vermelho para falha
        return false;
    }

    // Conexão bem-sucedida
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    printf("Wi-Fi conectado! IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    set_led_green_pwm(); // LED verde para conexão bem-sucedida
    return true;
}

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
#define SEA_LEVEL_PRESSURE 101700.0 // 101325.0 // Pressão ao nível do mar em Pa

// Tipos de dados
struct http_state
{
    char response[10000];
    size_t len;
    size_t sent;
};

// Prototipos
void get_simulated_data(AHT20_Data *data);
double calculate_altitude(double pressure);
void check_alerts(float temperature, float humidity);
void check_climate_conditions(float temperature, float humidity);
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void start_http_server(void);
void gpio_irq_handler(uint gpio, uint32_t events);

// Variáveis globais
static volatile int max_humidity_limit = 100; // Limite máximo de umidade
static volatile int min_humidity_limit = 0;  // Nível mínimo de umidade
static volatile int max_temperature_limit = 100; // Limite máximo de temperatura
static volatile int min_temperature_limit = 0; // Nível mínimo de temperatura
static volatile int64_t last_button_a_press_time = 0; // Tempo do último pressionamento de botão
static volatile int64_t last_button_b_press_time = 0; // Tempo do último pressionamento de botão B
static volatile int64_t last_button_sw_press_time = 0; // Tempo do último pressionamento do botão SW
static volatile bool is_alert_active = true; // Flag para indicar se o alerta está ativo
static volatile bool is_simulated = false; // Flag para simulação de dados

int main()
{
    stdio_init_all();

    init_btns();
    init_btn(BTN_SW_PIN);

    init_leds_pwm();

    init_joystick();

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
        printf("Falha ao inicializar a arquitetura CYW43\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Falha ao conectar ao WiFi\n");
    }

    // Printa o IP
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    char ip_str[24];
    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    printf("IP: %s\n", ip_str);
    start_http_server();

    // Estrutura para armazenar os dados do sensor
    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;
    int32_t temperature;
    int32_t pressure;
    double altitude;

    while (true)
    {
        cyw43_arch_poll();

        // Verifica se os dados estão sendo simulados
        if (is_simulated) {
            get_simulated_data(&data);
        } else {
            // Leitura do BMP280
            bmp280_read_raw(I2C0_PORT, &raw_temp_bmp, &raw_pressure);
            temperature = bmp280_convert_temp(raw_temp_bmp, &params);
            pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);

            // Cálculo da altitude
            altitude = calculate_altitude(pressure);

            printf("Pressao BMP = %.3f kPa\n", pressure / 1000.0);
            printf("Temperatura BMP: = %.2f C\n", temperature / 100.0);
            printf("Altitude estimada: %.2f m\n", altitude);

            // Leitura do AHT20
            if (aht20_read(I2C1_PORT, &data))
            {
                printf("Temperatura AHT: %.2f C\n", data.temperature);
                printf("Umidade: %.2f %%\n\n\n", data.humidity);
            }
            else
            {
                printf("Erro na leitura do AHT10!\n\n\n");
            }
        }

        // Verifica os alertas
        check_alerts(data.temperature, data.humidity);

        // Verifica as condições climáticas
        check_climate_conditions(data.temperature, data.humidity);

        sleep_ms(500);
    }
    cyw43_arch_deinit(); // Esperamos que nunca chegue aqui
}

// Função para calcular a altitude a partir da pressão atmosférica
double calculate_altitude(double pressure)
{
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

// Função para verificar os alertas de temperatura e umidade
void check_alerts(float temperature, float humidity) {
    if (is_alert_active) {
        if (temperature > max_temperature_limit || temperature < min_temperature_limit) {
            printf("Alerta: Temperatura fora dos limites!\n");
            play_tone(BUZZER_A_PIN, 1000); // Toca o buzzer A
            sleep_ms(250); //
            stop_tone(BUZZER_A_PIN); // Para o buzzer A
        }

        if (humidity > max_humidity_limit || humidity < min_humidity_limit) {
            printf("Alerta: Umidade fora dos limites!\n");
            play_tone(BUZZER_B_PIN, 2000); // Toca o buzzer B
            sleep_ms(250); // Espera 250ms
            stop_tone(BUZZER_B_PIN); // Para o buzzer B
        }
    }
}

// Função para verificar as condições climáticas
void check_climate_conditions(float temperature, float humidity) {
    bool clima_quente = temperature > 30;
    bool clima_frio   = temperature < 15;
    bool clima_umido  = humidity > 80;
    bool clima_seco   = humidity < 20;

    if (clima_quente && clima_umido) {
        printf("Clima quente e úmido: %.2f C / %.2f %%\n", temperature, humidity);
        set_led_orange_pwm(); //  quente + úmido
    }
    else if (clima_quente && clima_seco) {
        printf("Clima quente e seco: %.2f C / %.2f %%\n", temperature, humidity);
        set_led_purple_pwm(); //  quente + seco
    }
    else if (clima_frio && clima_umido) {
        printf("Clima frio e úmido: %.2f C / %.2f %%\n", temperature, humidity);
        set_led_cyan_pwm(); //  frio + úmido
    }
    else if (clima_frio && clima_seco) {
        printf("Clima frio e seco: %.2f C / %.2f %%\n", temperature, humidity);
        set_led_white_pwm(); // frio + seco
    }
    else if (clima_quente) {
        printf("Clima quente: %.2f C\n", temperature);
        set_led_red_pwm(); // Clima quente
    }
    else if (clima_frio) {
        printf("Clima frio: %.2f C\n", temperature);
        set_led_blue_pwm(); // Clima frio
    }
    else if (clima_umido) {
        printf("Clima úmido: %.2f %%\n", humidity);
        set_led_yellow_pwm(); // Clima úmido
    }
    else if (clima_seco) {
        printf("Clima seco: %.2f %%\n", humidity);
        set_led_magenta_pwm(); // Clima seco
    }
    else {
        printf("Clima ameno: %.2f C / %.2f %%\n", temperature, humidity);
        set_led_green_pwm(); // Clima ameno
    }
}

// Função para obter dados simulados do AHT20
void get_simulated_data(AHT20_Data *data) {
    // Simula dados de temperatura e umidade
    data->temperature = 100 - (get_joystick_y () / 4095.0 * 100.0); // Temperatura entre 0.0 e 100.0 C
    data->humidity = get_joystick_x () / 4095.0 * 100.0; // Umidade entre 0.0 e 100.0

    printf("Dados simulados: Temperatura: %.2f C, Umidade: %.2f %%\n", data->temperature, data->humidity);
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

    if (strstr(req, "POST /limites")) {
        char *body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;

            // Extrai os valores diretamente usando sscanf
            int max_val, min_val;
            if(sscanf(body, "{\"max\":%d,\"min\":%d", &max_val, &min_val) == 2) {
                // Valida os valores recebidos
                if(max_val >= 0 && max_val <= 100 && min_val >= 0 && min_val <= 100) {
                    max_humidity_limit = max_val;
                    min_humidity_limit = min_val;
                }
            }
        }

        printf("Novos limites: Max=%d, Min=%d\n",
            max_humidity_limit,
            min_humidity_limit);

        // Confirma atualização
        const char *txt = "Limites atualizados";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n"
                        "%s",
                        (int)strlen(txt), txt);
    }
    else{
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

        min_temperature_limit = 0; // Reseta o limite mínimo de temperatura
        max_temperature_limit = 100; // Reseta o limite máximo de temperatura
        min_humidity_limit = 0; // Reseta o limite mínimo de umidade
        max_humidity_limit = 100; // Reseta o limite máximo de umidade
    }

}

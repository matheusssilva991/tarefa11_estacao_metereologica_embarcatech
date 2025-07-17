#include "led.h"

void init_led(uint8_t pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

void init_leds()
{
    init_led(GREEN_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(RED_LED_PIN);
}

void init_led_pwm(uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint led_slice = pwm_gpio_to_slice_num(pin);


    pwm_set_wrap(led_slice, 255);   // 8-bit PWM (0-255)

    pwm_set_enabled(led_slice, true);
}

void init_leds_pwm()
{
    init_led_pwm(GREEN_LED_PIN);
    init_led_pwm(BLUE_LED_PIN);
    init_led_pwm(RED_LED_PIN);
}

void pwm_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    pwm_set_gpio_level(RED_LED_PIN, r);
    pwm_set_gpio_level(GREEN_LED_PIN, g);
    pwm_set_gpio_level(BLUE_LED_PIN, b);
}


void turn_off_leds()
{
    gpio_put(GREEN_LED_PIN, false);
    gpio_put(BLUE_LED_PIN, false);
    gpio_put(RED_LED_PIN, false);
}

void set_led_green()   { turn_off_leds; (GREEN_LED_PIN, true); }
void set_led_blue()    { turn_off_leds; gpio_put(BLUE_LED_PIN, true); }
void set_led_red()     { turn_off_leds; gpio_put(RED_LED_PIN, true); }
void set_led_yellow()  { turn_off_leds; gpio_put(GREEN_LED_PIN, true); gpio_put(BLUE_LED_PIN, true); } // Verde + Azul
void set_led_orange()  { turn_off_leds; gpio_put(GREEN_LED_PIN, true); gpio_put(BLUE_LED_PIN, false); gpio_put(RED_LED_PIN, true); } // Verde + Vermelho
void set_led_cyan()    { turn_off_leds; gpio_put(GREEN_LED_PIN, true); gpio_put(BLUE_LED_PIN, true); gpio_put(RED_LED_PIN, false); } // Verde + Azul
void set_led_white()   { turn_off_leds; gpio_put(GREEN_LED_PIN, true); gpio_put(BLUE_LED_PIN, true); gpio_put(RED_LED_PIN, true); } // Todas cores
void set_led_off()     { turn_off_leds; gpio_put(GREEN_LED_PIN, false); gpio_put(BLUE_LED_PIN, false); gpio_put(RED_LED_PIN, false); } // Desliga

void set_led_red_pwm ()    { pwm_set_rgb(255, 0, 0); }
void set_led_blue_pwm ()   { pwm_set_rgb(0, 0, 255); }
void set_led_green_pwm ()  { pwm_set_rgb(0, 255, 0); }
void set_led_yellow_pwm () { pwm_set_rgb(255, 255, 0); }  // Vermelho + Verde
void set_led_orange_pwm () { pwm_set_rgb(255, 100, 0); }  // Laranja = V + G reduzido
void set_led_cyan_pwm ()   { pwm_set_rgb(0, 255, 255); }  // Verde + Azul
void set_led_white_pwm ()  { pwm_set_rgb(255, 255, 255); } // Todas cores
void set_led_off_pwm ()    { pwm_set_rgb(0, 0, 0); }       // Desliga


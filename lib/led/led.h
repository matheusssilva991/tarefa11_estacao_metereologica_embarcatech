#ifndef LED_H
#define LED_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define GREEN_LED_PIN 11 // GPIO para LED verde
#define BLUE_LED_PIN 12  // GPIO para LED azul
#define RED_LED_PIN 13   // GPIO para LED vermelho

void init_led(uint8_t pin);
void init_leds();
void turn_off_leds();
void set_led_green();
void set_led_blue();
void set_led_red();
void set_led_yellow();
void set_led_orange();
void set_led_cyan();
void set_led_white();
void set_led_off();
void set_led_magenta();
void set_led_purple();

void init_led_pwm(uint8_t pin);
void init_leds_pwm();
void set_led_red_pwm();
void set_led_blue_pwm();
void set_led_green_pwm();
void set_led_yellow_pwm();
void set_led_orange_pwm();
void set_led_cyan_pwm();
void set_led_white_pwm();
void set_led_off_pwm();
void set_led_magenta_pwm();
void set_led_purple_pwm();
void pwm_set_rgb(uint8_t r, uint8_t g, uint8_t b);

#endif // LED_H

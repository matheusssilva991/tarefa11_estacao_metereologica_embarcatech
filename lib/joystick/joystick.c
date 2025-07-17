#include "joystick.h"


void init_joystick() {
    adc_init();
    adc_gpio_init(JRY_PIN);
    adc_gpio_init(JRX_PIN);
}

int get_joystick_x() {
    adc_select_input(1); // Seleciona o canal 0 para o eixo X
    return adc_read();
}

int get_joystick_y() {
    adc_select_input(0); // Seleciona o canal 1 para o eixo Y
    return adc_read();
}

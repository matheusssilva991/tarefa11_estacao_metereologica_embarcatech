#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define JRY_PIN 26 // GPIO para o eixo Y
#define JRX_PIN 27 // GPIO para o eixo X

void init_joystick();
int get_joystick_x();
int get_joystick_y();

#endif // JOYSTICK_H

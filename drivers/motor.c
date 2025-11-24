#include "motor.h"
#include <avr/io.h>

void motor_init(void) {
    DDRD |= _BV(PD3);     // OC2B en sortie

    // Timer2 en Fast PWM mode
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS21);   // prescaler 8 → PWM stable
}

void motor_set_speed(uint8_t duty) {
    OCR2B = duty;
}

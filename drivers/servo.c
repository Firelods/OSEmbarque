#include "servo.h"
#include <avr/io.h>

void servo_init(void)
{
    // OC1A = PB1 = Arduino D9
    DDRB |= (1 << PB1);

    // Fast PWM mode, TOP = ICR1
    TCCR1A = (1 << COM1A1);                 // non-inverted PWM on OC1A
    TCCR1B = (1 << WGM13) | (1 << WGM12);   // Fast PWM mode
    TCCR1A |= (1 << WGM11);

    // 50 Hz: ICR1 = 20ms / 0.5us = 40000
    ICR1 = 40000;

    // Prescaler 8
    TCCR1B |= (1 << CS11);

    // Default angle = UP
    servo_set_angle(0);
}

void servo_set_angle(uint8_t angle)
{
    // Map angle (0–180) to pulse width (1ms–2ms)
    // 1ms = 2000 ticks, 2ms = 4000 ticks
    uint16_t pulse = 2000 + (angle * 2000UL) / 180;

    OCR1A = pulse;
}

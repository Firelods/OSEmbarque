#include "servo.h"
#include <avr/io.h>

/*
 * Servo sur D6 (PD6, OC0A)
 * Timer0 → Fast PWM, TOP = 0xFF, prescaler = 1024
 * 
 * Période PWM ≈ 16.384 ms (~61 Hz)
 * Résolution ≈ 64 µs par tick.
 */

void servo_init(void)
{
    // D6 = PD6 = OC0A en sortie
    DDRD |= (1 << PD6);

    // Fast PWM, TOP = 0xFF
    // WGM01=1, WGM00=1, WGM02=0
    TCCR0A = (1 << WGM01) | (1 << WGM00);

    // Non-inverted PWM sur OC0A : Clear OC0A on compare match
    TCCR0A |= (1 << COM0A1);

    // Prescaler = 1024 → CS02=1, CS01=0, CS00=1
    TCCR0B = (1 << CS02) | (1 << CS00);

    // Position de repos : barrière ouverte (0°)
    servo_set_angle(0);
}

void servo_set_angle(uint16_t angle)
{
    // Clamp au cas où
    // if (angle > 1080) angle = 1080;

    /*
     * On prend :
     *  - ~0.5 ms → 500 / 64 ≈ 8 ticks
     *  - ~2.5 ms → 2500 / 64 ≈ 39 ticks
     * On mappe 0–1080° → 8–39
     */

    const uint8_t pulse_min = 8;    // ~0.5 ms
    const uint8_t pulse_max = 39;   // ~2.5 ms

    uint8_t ocr = pulse_min + (uint32_t)(pulse_max - pulse_min) * angle / 1080;

    OCR0A = ocr;
}

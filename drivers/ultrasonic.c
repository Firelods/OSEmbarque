#include "ultrasonic.h"
#include <avr/io.h>
#include <util/delay.h>

#define TRIGGER PB1
#define ECHO    PB0

void us_init(void) {
    DDRB |= _BV(TRIGGER);
    DDRB &= ~_BV(ECHO);

    // Timer1 en mode normal
    TCCR1A = 0;
    TCCR1B = _BV(CS11); // prescaler 8 → précision suffisante
}

uint16_t us_measure_cm(void) {
    // Pulse trigger 10µs
    PORTB &= ~_BV(TRIGGER);
    _delay_us(2);
    PORTB |= _BV(TRIGGER);
    _delay_us(10);
    PORTB &= ~_BV(TRIGGER);

    // Attente FRONT MONTANT
    while (!(PINB & _BV(ECHO)));
    TCNT1 = 0;

    // Attente FRONT DESCENDANT
    while (PINB & _BV(ECHO));
    uint16_t ticks = TCNT1;

    // Conversion en cm (voir datasheet HC-SR04)
    return ticks / 58;
}

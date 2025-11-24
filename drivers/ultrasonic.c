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

    // Attente FRONT MONTANT (Timeout ~few ms)
    // Increased timeout to ensure we don't miss the start
    uint32_t timeout = 500000; 
    while (!(PINB & _BV(ECHO))) {
        if (--timeout == 0) return 0xFFFF; // Error: Echo never went HIGH
    }

    TCNT1 = 0;

    // Attente FRONT DESCENDANT (Timeout ~30ms for max range)
    // Prescaler 8 @ 16MHz => 0.5us per tick. 60000 ticks = 30ms.
    while (PINB & _BV(ECHO)) {
        if (TCNT1 > 60000) return 0xFFFE; // Error: Echo stayed HIGH too long
    }
    
    uint16_t ticks = TCNT1;

    // Conversion en cm
    // Prescaler 8 => 1 tick = 0.5us
    // Time (us) = ticks * 0.5
    // Distance (cm) = Time (us) / 58 = ticks / 116
    return ticks / 116;
}

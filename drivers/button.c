#include "button.h"
#include <avr/interrupt.h>
#include <util/delay.h>

#define BUTTON_PIN PD4

volatile static uint8_t event = 0;

void button_init(void) {
    DDRD &= ~_BV(BUTTON_PIN);
    PORTD |= _BV(BUTTON_PIN);

    PCICR  |= _BV(PCIE2);
    PCMSK2 |= _BV(BUTTON_PIN);
}

uint8_t button_get_event(void) {
    if (event) {
        event = 0;
        return 1;
    }
    return 0;
}

ISR(PCINT2_vect)
{
    _delay_ms(20);
    if (!(PIND & _BV(BUTTON_PIN)))
        event = 1;
}

#include "led.h"

#define LED_PIN  PD2

void led_init(void) {
    DDRD |= _BV(LED_PIN);
}

void led_set(uint8_t state) {
    if (state) PORTD |= _BV(LED_PIN);
    else       PORTD &= ~_BV(LED_PIN);
}

void led_toggle(void) {
    PORTD ^= _BV(LED_PIN);
}

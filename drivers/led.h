#ifndef LED_H
#define LED_H

#include <avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_init(void);
void led_set(uint8_t state);
void led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif

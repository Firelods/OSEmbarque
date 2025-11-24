#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

void button_init(void);
uint8_t button_get_event(void);

#ifdef __cplusplus
}
#endif

#endif

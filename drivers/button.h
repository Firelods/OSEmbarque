#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>

void button_init(void);
uint8_t button_get_event(void);

#endif

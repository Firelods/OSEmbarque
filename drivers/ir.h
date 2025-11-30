#ifndef IR_H
#define IR_H

#include <avr/io.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ir_init(void);         // Configure IR sensor pin
uint8_t ir_detect(void);    // Returns 1 if obstacle detected, 0 otherwise

#ifdef __cplusplus
}
#endif

#endif

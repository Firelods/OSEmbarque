#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_set_speed(uint8_t duty);  // 0–255

#endif

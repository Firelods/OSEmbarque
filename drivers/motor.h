#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void motor_init(void);
void motor_set_speed(uint8_t duty);  // 0–255

#ifdef __cplusplus
}
#endif

#endif

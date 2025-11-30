#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void servo_init(void);
void servo_set_angle(uint8_t angle);   // 0–180°

#ifdef __cplusplus
}
#endif

#endif

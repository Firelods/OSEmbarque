#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void us_init(void);
uint16_t us_measure_cm(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

void us_init(void);
uint16_t us_measure_cm(void);

#endif

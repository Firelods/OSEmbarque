#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void soft_i2c_init(uint8_t address);

uint8_t soft_i2c_get_register(uint8_t reg);
void    soft_i2c_set_register(uint8_t reg, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif

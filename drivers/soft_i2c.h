#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialise le bus I2C en mode slave
// Note: Utilise les pins A4 (SDA) et A5 (SCL) - pins I2C matérielles
void soft_i2c_init(uint8_t address);

// Lit un registre I2C (0-15)
uint8_t soft_i2c_get_register(uint8_t reg);

// Écrit dans un registre I2C (0-15)
void    soft_i2c_set_register(uint8_t reg, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif

#include "soft_i2c.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define SDA_PIN  PB1    // Pin 9
#define SCL_PIN  PB2    // Pin 10

#define SDA_READ   (PINB & (1<<SDA_PIN))
#define SCL_READ   (PINB & (1<<SCL_PIN))

static volatile uint8_t i2c_addr = 0x32;
static volatile uint8_t registers[16];
static volatile uint8_t reg_index = 0;

void soft_i2c_set_register(uint8_t reg, uint8_t value)
{
    if (reg < sizeof(registers))
        registers[reg] = value;
}

uint8_t soft_i2c_get_register(uint8_t reg)
{
    if (reg < sizeof(registers))
        return registers[reg];
    return 0xFF;
}

void soft_i2c_init(uint8_t address)
{
    i2c_addr = address;

    // SDA & SCL inputs with pull-up
    DDRB &= ~((1<<SDA_PIN) | (1<<SCL_PIN));
    PORTB |= (1<<SDA_PIN) | (1<<SCL_PIN);

    // Enable Pin Change Interrupt on PB1 and PB2
    PCICR  |= (1<<PCIE0);              // enable PCINT for port B
    PCMSK0 |= (1<<PCINT1) | (1<<PCINT2);

    sei();
}

/*
 * Very lightweight I2C slave handler
 * Detects:  START → address → RW → register → data
 */
ISR(PCINT0_vect)
{
    static uint8_t bit_count = 0;
    static uint8_t byte_value = 0;
    static uint8_t receiving = 0;
    static uint8_t rw = 0;

    // Detect clock rising edge
    if (SCL_READ)
    {
        uint8_t sda = SDA_READ ? 1 : 0;

        byte_value = (byte_value << 1) | sda;
        bit_count++;

        if (bit_count == 8)
        {
            bit_count = 0;

            if (!receiving)
            {
                // First byte = address
                rw = (byte_value & 1);
                if ((byte_value >> 1) == i2c_addr)
                {
                    receiving = 1;
                }
            }
            else
            {
                // Register index then data
                if (rw == 0)
                {
                    // Write from master
                    registers[reg_index] = byte_value;
                    reg_index++;
                }
                else
                {
                    // Read to master
                    uint8_t out = registers[reg_index];
                    reg_index++;
                    // Slave drives SDA here (not implemented to keep it light)
                }
            }

            byte_value = 0;
        }
    }
}

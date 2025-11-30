#include "lcd_grove.h"

#include <avr/io.h>
#include <util/delay.h>

// ----------------------------
// Pins (Arduino UNO / ATmega328P)
// SDA = A4 = PC4
// SCL = A5 = PC5
// ----------------------------
#define SDA_BIT PC4
#define SCL_BIT PC5

// Grove LCD I2C address
#define LCD_ADDR 0x3E

// LCD commands (HD44780-like controller)
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20
#define LCD_SETDDRAMADDR   0x80

// Flags for display entry mode
#define LCD_ENTRYLEFT          0x02
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSOROFF  0x00
#define LCD_BLINKOFF   0x00

// Flags for function set
#define LCD_2LINE   0x08
#define LCD_5x8DOTS 0x00

// ----------------------------
// Software I2C helpers
// ----------------------------

static void i2c_delay(void)
{
    // ~5us delay → ~100 kHz on the bus
    _delay_us(5);
}

static void sda_release(void)
{
    // input + pull-up = logical HIGH (lines are open-drain)
    DDRC &= ~(1 << SDA_BIT);
    PORTC |= (1 << SDA_BIT);
}

static void sda_low(void)
{
    // drive low
    DDRC |= (1 << SDA_BIT);
    PORTC &= ~(1 << SDA_BIT);
}

static void scl_release(void)
{
    DDRC &= ~(1 << SCL_BIT);
    PORTC |= (1 << SCL_BIT);
}

static void scl_low(void)
{
    DDRC |= (1 << SCL_BIT);
    PORTC &= ~(1 << SCL_BIT);
}

static void i2c_start(void)
{
    sda_release();
    scl_release();
    i2c_delay();

    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

static void i2c_stop(void)
{
    sda_low();
    i2c_delay();
    scl_release();
    i2c_delay();
    sda_release();
    i2c_delay();
}

static void i2c_write_byte(uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (b & 0x80) {
            sda_release();   // 1
        } else {
            sda_low();       // 0
        }

        i2c_delay();
        scl_release();       // clock high
        i2c_delay();
        scl_low();           // clock low
        i2c_delay();

        b <<= 1;
    }

    // ACK bit (we ignore it but must still clock it)
    sda_release();           // release SDA for slave to drive
    i2c_delay();
    scl_release();
    i2c_delay();
    scl_low();
    i2c_delay();
}

// ----------------------------
// LCD low-level helpers
// ----------------------------

static void lcd_send_command(uint8_t cmd)
{
    i2c_start();
    // SLA+W
    i2c_write_byte((LCD_ADDR << 1) | 0x00);
    // Control byte: 0x80 = "next byte is command"
    i2c_write_byte(0x80);
    // Command
    i2c_write_byte(cmd);
    i2c_stop();
}

static void lcd_send_data(uint8_t data)
{
    i2c_start();
    // SLA+W
    i2c_write_byte((LCD_ADDR << 1) | 0x00);
    // Control byte: 0x40 = "next byte is data"
    i2c_write_byte(0x40);
    // Data
    i2c_write_byte(data);
    i2c_stop();
}

// ----------------------------
// Public API
// ----------------------------

void lcd_init(void)
{
    // Release lines + enable pull-ups
    sda_release();
    scl_release();

    _delay_ms(50);  // power-on wait

    // Function set: 2 lines, 5x8 dots
    lcd_send_command(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS);
    _delay_ms(5);

    // Display ON, cursor OFF, blink OFF
    lcd_send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    _delay_ms(5);

    // Clear display
    lcd_send_command(LCD_CLEARDISPLAY);
    _delay_ms(5);

    // Set entry mode: left-to-right, no shift
    lcd_send_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    _delay_ms(5);
}

void lcd_clear(void)
{
    lcd_send_command(LCD_CLEARDISPLAY);
    _delay_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    if (row > 1) row = 1;
    uint8_t addr = (row == 0 ? 0x00 : 0x40) + col;
    lcd_send_command(LCD_SETDDRAMADDR | addr);
    _delay_us(50);
}

void lcd_print(const char *s)
{
    while (*s) {
        lcd_send_data((uint8_t)*s++);
        _delay_us(50);
    }
}

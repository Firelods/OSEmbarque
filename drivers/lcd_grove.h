#ifndef LCD_GROVE_H
#define LCD_GROVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *s);

#ifdef __cplusplus
}
#endif

#endif

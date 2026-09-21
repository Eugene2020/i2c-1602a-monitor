#ifndef I2C_LCD_H
#define I2C_LCD_H

#include <stdint.h>

#define LCD_I2C_ADDR    0x20

void lcd_init(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_clear(void);
void lcd_print(const char *str);
void lcd_putchar(char c);
void lcd_backlight(uint8_t on);

#endif
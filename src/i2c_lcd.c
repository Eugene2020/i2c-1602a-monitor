#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "i2c_lcd.h"

#define LCD_RS          (1 << 0)
#define LCD_RW          (1 << 1)
#define LCD_EN          (1 << 2)
#define LCD_BACKLIGHT   (1 << 3)

static void i2c_write_byte(uint8_t data);

static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble << 4) | LCD_BACKLIGHT;
    if (rs) data |= LCD_RS;

    i2c_write_byte(data);
    i2c_write_byte(data | LCD_EN);
    i2c_write_byte(data);
}

static void lcd_write_byte(uint8_t value, uint8_t rs)
{
    lcd_write_nibble(value >> 4, rs);
    lcd_write_nibble(value & 0x0F, rs);
}

static void lcd_write_cmd(uint8_t cmd)
{
    lcd_write_byte(cmd, 0);
}

static void lcd_write_data(uint8_t data)
{
    lcd_write_byte(data, 1);
}

void lcd_init(void)
{
    for (volatile int i = 0; i < 1000000; i++);

    lcd_write_nibble(0x03, 0);
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_nibble(0x03, 0);
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_nibble(0x03, 0);
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_nibble(0x02, 0);  

    lcd_write_cmd(0x28);  
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_cmd(0x0C);  
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_cmd(0x06);  
    for (volatile int i = 0; i < 500000; i++);
    lcd_write_cmd(0x01);  
    for (volatile int i = 0; i < 500000; i++);
}

void lcd_clear(void)
{
    lcd_write_cmd(0x01);
    for (volatile int i = 0; i < 500000; i++);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    lcd_write_cmd(0x80 | (addr + col));
}

void lcd_putchar(char c)
{
    lcd_write_data((uint8_t)c);
}

void lcd_print(const char *str)
{
    while (*str) {
        lcd_putchar(*str++);
    }
}

void lcd_backlight(uint8_t on)
{
    uint8_t data = on ? LCD_BACKLIGHT : 0;
    i2c_write_byte(data);
}

static void i2c_write_byte(uint8_t data)
{
    while (I2C_SR2(I2C1) & I2C_SR2_BUSY);

    i2c_send_start(I2C1);
    while (!((I2C_SR1(I2C1) & I2C_SR1_SB) &&
             (I2C_SR2(I2C1) & I2C_SR2_MSL)));

    i2c_send_7bit_address(I2C1, LCD_I2C_ADDR, I2C_WRITE);
    while (!(I2C_SR1(I2C1) & I2C_SR1_ADDR));
    (void)I2C_SR1(I2C1);
    (void)I2C_SR2(I2C1);

    i2c_send_data(I2C1, data);
    while (!(I2C_SR1(I2C1) & I2C_SR1_TxE));

    i2c_send_stop(I2C1);

    for (volatile int i = 0; i < 100; i++);
}
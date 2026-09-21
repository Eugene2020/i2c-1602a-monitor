#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/adc.h>
#include <stdio.h>
#include <string.h>
#include "i2c_lcd.h"
#include "uptime.h"

static void i2c_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_I2C1);
    rcc_periph_clock_enable(RCC_AFIO);

    rcc_periph_reset_pulse(RST_I2C1);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
                  GPIO_I2C1_SCL | GPIO_I2C1_SDA);

    i2c_peripheral_disable(I2C1);
    i2c_set_clock_frequency(I2C1, 36);   
    i2c_set_ccr(I2C1, 180);              
    i2c_set_trise(I2C1, 37);
    i2c_peripheral_enable(I2C1);
}

static void adc_temp_setup(void)
{
    rcc_periph_clock_enable(RCC_ADC1);

    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_239DOT5CYC);
    adc_power_on(ADC1);

    adc_enable_temperature_sensor();

    for (volatile int i = 0; i < 100000; i++);

    adc_reset_calibration(ADC1);
    while (adc_is_calibrating(ADC1));

    adc_calibrate(ADC1);
    while (adc_is_calibrating(ADC1));
}

static float read_internal_temp(void)
{
    uint8_t channel = 16;
    adc_set_regular_sequence(ADC1, 1, &channel);

    adc_start_conversion_regular(ADC1);

    uint32_t timeout = 1000000;
    while (!adc_eoc(ADC1) && timeout--);

    if (timeout == 0) {
        return -273.0f; 
    }

    uint16_t adc_value = adc_read_regular(ADC1);

    float voltage = (adc_value * 3.3f) / 4096.0f;
    float temperature = (1.43f - voltage) / 0.0043f + 25.0f;

    return temperature;
}

int main(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);

    uptime_init();
    i2c_setup();
    lcd_init();
    adc_temp_setup();

    char line1[17];
    char line2[17];
    uint32_t last_uptime = 0;
    int first_run = 1;

    while (1) {
        uint32_t current_uptime = uptime_get_seconds();

        if (first_run || current_uptime != last_uptime) {
            first_run = 0;
            last_uptime = current_uptime;

            snprintf(line1, sizeof(line1), "Uptime: %lu s", current_uptime);
            lcd_set_cursor(0, 0);
            lcd_print(line1);

            float temp = read_internal_temp();
            snprintf(line2, sizeof(line2), "Temp: %.1f C", (double)temp);
            lcd_set_cursor(0, 1);
            lcd_print(line2);
        }
    }

    return 0;
}
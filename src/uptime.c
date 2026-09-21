#include <libopencm3/cm3/systick.h>
#include "uptime.h"

static volatile uint32_t tick_counter = 0;

void sys_tick_handler(void)
{
    tick_counter++;
}

void uptime_init(void)
{
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    systick_set_reload(9000 - 1);
    systick_interrupt_enable();
    systick_counter_enable();
}

uint32_t uptime_get_seconds(void)
{
    return tick_counter / 1000;
}
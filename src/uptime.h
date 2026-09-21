#ifndef UPTIME_H
#define UPTIME_H

#include <stdint.h>

void uptime_init(void);
uint32_t uptime_get_seconds(void);

#endif
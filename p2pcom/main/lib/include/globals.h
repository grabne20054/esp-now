#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include <stdbool.h>
#include "send.h"

#if SWITCH == 1
extern EventGroupHandle_t channel_hopping;
#endif

#endif // GLOBALS_H
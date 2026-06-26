#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdbool.h>

#include "send.h"

extern volatile uint32_t global_seq;
extern volatile bool waiting_for_recv;

uint32_t crc32(const void *data, size_t length);

#endif
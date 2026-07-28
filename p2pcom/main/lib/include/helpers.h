#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdbool.h>

#include "send.h"

extern volatile uint32_t global_seq;
extern volatile bool waiting_for_recv;
extern volatile bool data_received; // Flag to indicate if data has been received (with response status) --> do not transmit again

uint32_t crc32(const void *data, size_t length);

uint32_t get_max_clock_value(uint32_t current_value, uint32_t new_value);

char * data_str_repr(data_stream_t * data);

char * command_str_repr(e_actions_t command);

#endif
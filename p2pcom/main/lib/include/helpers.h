#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdbool.h>

#include "send.h"

extern volatile uint32_t global_seq;
extern volatile bool waiting_for_recv;


// testing section
extern volatile uint32_t add_queue_counter;
extern volatile uint32_t failed_queue_counter;

extern volatile uint32_t send_counter;
extern volatile uint32_t failed_send_counter;

extern volatile uint32_t recv_counter;

extern volatile uint32_t recv_wrong_crc;
extern volatile uint32_t recv_wrong_action;
extern volatile uint32_t recv_not_waiting;



uint32_t crc32(const void *data, size_t length);

uint32_t get_max_clock_value(uint32_t current_value, uint32_t new_value);

#endif
#ifndef HELPERS_C
#define HELPERS_C

#include "../include/helpers.h"

#include <stdlib.h>
#include "esp_log.h"

static const char *TAG_HELPERS = "helpers";

// global var for communication
volatile uint32_t global_seq;
volatile bool waiting_for_recv;
volatile bool data_received;

#if SWITCH == 1
EventGroupHandle_t channel_hopping = NULL;
#endif

uint32_t crc32(const void *data, size_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= bytes[i];

        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}


uint32_t get_max_clock_value(uint32_t current_value, uint32_t new_value)
{
    if (new_value > current_value)
    {
        return new_value;
    }
    else
    {
        return current_value;
    }
}

char * data_str_repr(data_stream_t * data)
{
    char * repr = malloc(256);
    snprintf(repr, 256, "Data Stream: {command: %d, action: %d, seq: %ld, sent: %ld, ttl: %ld, crc: %lu}", 
             data->command, data->action, data->seq, data->sent, data->ttl, data->crc);
    return repr;
}

#endif
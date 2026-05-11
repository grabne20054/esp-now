#ifndef HELPERS_C
#define HELPERS_C

#include <stdlib.h>

static const char *TAG_HELPERS = "helpers";

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

    ESP_LOGI(TAG_HELPERS, "CRC calculated" );

    return ~crc;
}


#endif
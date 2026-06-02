#ifndef SEND_C
#define SEND_C

#include "../include/send.h"

static data_stream_t * prepare_data_stream(e_actions_t action)
{
    uint8_t dest_mac[6] = PEER_MAC_ADDR;
    time_t raw_time;
    data_stream_t * stream = malloc(sizeof(data_stream_t));

    stream->command=action;
    memcpy(stream->dest, dest_mac, 6);
    stream->ttl=MAX_TTL;

    stream->engine = NULL;

    stream->sent=time(&raw_time);
    stream->crc = 0;

    uint32_t crc = crc32(stream, (sizeof(*stream)));
    stream->crc = crc;

    return stream;

}


bool add_to_queue(e_actions_t action, queue_t *unq_queue)
{
    data_stream_t * stream = prepare_data_stream(action);

    return enqueue(unq_queue, (void*)stream);
}

esp_err_t transmit(data_stream_t * stream)
{
    esp_now_send(stream->dest, (uint8_t*)stream, sizeof(*stream));

    return ESP_OK;
}



#endif
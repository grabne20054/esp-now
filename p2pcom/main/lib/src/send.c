#ifndef SEND_C
#define SEND_C

#include "../include/send.h"

data_stream_t * prepare_data_stream(e_actions_t command, ds_actions_t action)
{
    uint8_t dest_mac[6] = PEER_MAC_ADDR;
    time_t raw_time;
    data_stream_t * stream = calloc(1, sizeof(data_stream_t));
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "calloc failed");
        return NULL;
    }

    stream->seq=0;
    stream->command=command;
    stream->action=action;
    memcpy(stream->dest, dest_mac, 6);
    stream->ttl=MAX_TTL;

    stream->sent=time(&raw_time);
    stream->crc = 0;

    return stream;

}


bool add_action_to_queue(e_actions_t command, ds_actions_t action, queue_t *unq_queue)
{

    data_stream_t * stream = prepare_data_stream(command, action);
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "prepare_data_stream failed");
        return false;
    }

    bool add_succ = enqueue(unq_queue, stream);

    ESP_LOGI(SEND_TAG, "initial add_succ: %d", add_succ);

    return add_succ;
}

bool add_data_stream_to_queue(data_stream_t * stream, queue_t *unq_queue)
{
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "prepare_data_stream failed");
        return false;
    }

    bool add_succ = enqueue(unq_queue, stream);

    ESP_LOGI(SEND_TAG, "initial add_succ: %d", add_succ);

    return add_succ;

}

esp_err_t transmit(data_stream_t stream)
{
    uint32_t crc = crc32(&stream, (sizeof(stream)));
    stream.crc = crc;
    ESP_LOGI(SEND_TAG, "Peer added successfully: [" MACSTR "]", MAC2STR(stream.dest));

    esp_err_t res = esp_now_send(stream.dest, (uint8_t*)&stream, sizeof(stream));
    
    if (stream.action == REQUEST)
    {
        waiting_for_recv = false;
    }

    return res;
}



#endif
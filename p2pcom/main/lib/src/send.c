#ifndef SEND_C
#define SEND_C

#include "../include/send.h"

data_stream_t * prepare_data_stream(e_actions_t action)
{
    uint8_t dest_mac[6] = PEER_MAC_ADDR;
    time_t raw_time;
    data_stream_t * stream = malloc(sizeof(data_stream_t));
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "malloc failed");
        return NULL;
    }

    stream->command=action;
    memcpy(stream->dest, dest_mac, 6);
    ESP_LOGI(SEND_TAG, "Datastream copy succ: [" MACSTR "]", MAC2STR(stream->dest));
    stream->ttl=MAX_TTL;

    if (action!=RESPONSE)
    {
        stream->engine = NULL;
    }


    stream->sent=time(&raw_time);
    stream->crc = 0;

    uint32_t crc = crc32(stream, (sizeof(*stream)));
    stream->crc = crc;

    ESP_LOGI(SEND_TAG, "prepare data succ");

    return stream;

}


bool add_action_to_queue(e_actions_t action, queue_t *unq_queue)
{

    data_stream_t * stream = prepare_data_stream(action);
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "prepare_data_stream failed");
        return false;
    }

    bool add_succ = enqueue(unq_queue, stream);

    ESP_LOGI(SEND_TAG, "initial add_succ: %d", add_succ);

    const int max_attempts = 5;
    int attempts = 0;

    while (!add_succ && attempts < max_attempts)
    {
       vTaskDelay(pdMS_TO_TICKS(1000));
       attempts++;

       ESP_LOGI(SEND_TAG, "trying add queue (attempt %d)", attempts);

       add_succ = enqueue(unq_queue, stream);

       ESP_LOGI(SEND_TAG, "add_succ: %d", add_succ );
    }

    if (!add_succ) {
        ESP_LOGE(SEND_TAG, "failed to add to queue after %d attempts", attempts);
        free(stream);
    }

    ESP_LOGI(SEND_TAG, "returning from add_to_queue");
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

    const int max_attempts = 5;
    int attempts = 0;

    while (!add_succ && attempts < max_attempts)
    {
       vTaskDelay(pdMS_TO_TICKS(1000));
       attempts++;

       ESP_LOGI(SEND_TAG, "trying add queue (attempt %d)", attempts);

       add_succ = enqueue(unq_queue, stream);

       ESP_LOGI(SEND_TAG, "add_succ: %d", add_succ );
    }

    if (!add_succ) {
        ESP_LOGE(SEND_TAG, "failed to add to queue after %d attempts", attempts);
        free(stream);
    }

    ESP_LOGI(SEND_TAG, "returning from add_to_queue");
    return add_succ;

}

esp_err_t transmit(data_stream_t *stream)
{
    if (stream == NULL) {
        ESP_LOGE(SEND_TAG, "transmit: stream is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    global_seq++;

    stream->seq = global_seq;
    ESP_LOGI(SEND_TAG, "Peer added successfully: [" MACSTR "]", MAC2STR(stream->dest));
    
    esp_err_t res = esp_now_send(stream->dest, (uint8_t*)stream, sizeof(*stream));
    
    waiting_for_recv=true;

    return res;
}



#endif
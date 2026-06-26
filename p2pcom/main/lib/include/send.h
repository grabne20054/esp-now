#ifndef SEND_H
#define SEND_H

#include "espnow.h"
#include "espnow_storage.h"
#include "espnow_utils.h"
#include <pthread.h>

#include "data.h"
#include "../include/helpers.h"
#include "data_queue.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif

#define SWITCH 1 // 0=ws side   1=field side

#define PEER_MAC_ADDR_WS {0xd4, 0xe9, 0xf4, 0xfb, 0x4a, 0x6c} // peer addr not own mac
#define PEER_MAC_ADDR_FIELD {0xd4, 0xe9, 0xf4, 0xfb, 0x0a, 0x64} // peer addr not own mac

#if SWITCH == 0
#define PEER_MAC_ADDR PEER_MAC_ADDR_WS
#else
#define PEER_MAC_ADDR PEER_MAC_ADDR_FIELD
#endif
#define MAX_TTL 100

#define add_to_queue(X, Q) _Generic((X), \
    e_actions_t: add_action_to_queue,            \
    data_stream_t *: add_data_stream_to_queue,  \
    default: add_action_to_queue               \
)(X, Q)

static const char *SEND_TAG = "send";

data_stream_t * prepare_data_stream(e_actions_t action);

bool add_action_to_queue(e_actions_t action, queue_t * unq_queue);
bool add_data_stream_to_queue(data_stream_t * stream, queue_t * unq_queue);

esp_err_t transmit(data_stream_t *stream);


#endif
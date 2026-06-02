#ifndef SEND_H
#define SEND_H

#include "espnow.h"
#include "espnow_storage.h"
#include "espnow_utils.h"

#include "data.h"
#include "../src/helpers.c"
#include "queue.h"

#define PEER_MAC_ADDR {0xd4, 0xe9, 0xf4, 0xfb, 0x4a, 0x6c}  // placeholder
#define MAX_TTL 100



static data_stream_t * prepare_data_stream(e_actions_t action);

bool add_to_queue(e_actions_t action, queue_t * unq_queue);

esp_err_t transmit(data_stream_t * stream);


#endif
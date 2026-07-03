#ifndef QUEUE_HPP
#define QUEUE_HPP
/*
Header File for Queuing Transmit Request 
*/

#include "data.h"
#include <stdbool.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_mac.h"

#include "../include/engine.h"

#include "freertos/FreeRTOS.h"


#define MAX_QUEUE_SIZE 7
static const char *QUEUE_TAG = "queue";

queue_t * create(int max_size);

bool enqueue(queue_t * instance, data_stream_t * data_stream);

data_stream_t dequeue(queue_t * instance);

bool is_empty(queue_t * instance);

void destroy(queue_t * instance);

queue_t * get_unq_queue();

bool test_queue();

void do_queue(void *pvParameters);


#endif
#ifndef QUEUE_HPP
#define QUEUE_HPP
/*
Header File for Queuing Transmit Request 
*/

#include "data.h"
#include <stdbool.h>
#include <stdlib.h>

#define MAX_QUEUE_SIZE 7

queue_t * create(int max_size);

bool enqueue(queue_t * instance, void * data_stream_t);

void * dequeue(queue_t * instance);

bool is_empty(queue_t * instance);

void destroy(queue_t * instance);

queue_t * get_unq_queue();

bool test_queue();


#endif
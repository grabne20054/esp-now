#ifndef QUEUE_C
#define QUEUE_C

#include "../include/queue.h"

queue_t * create(int max_size)
{
    if (max_size <= 0)
    {
        return NULL;
    }

    queue_t * instance = malloc(sizeof(queue_t));

    instance->max_size = max_size;
    instance->front = 0;
    instance->current_size = 0;
    instance->data_stream = malloc(instance->max_size * sizeof(void*));

    return instance;

}


bool enqueue(queue_t * instance, void * data_stream)
{
    if ((instance->current_size == instance->max_size) || (data_stream == NULL))
    {
        return false;
    } else {
        instance->rear = (instance->rear + 1 ) % instance->max_size;
        ((void**) instance->data_stream)[instance->rear] = data_stream;
        instance->current_size++;
        return true;
    }
    
}

bool is_empty(queue_t * instance)
{
    return (instance->current_size == 0);
}

void * dequeue(queue_t * instance)
{
    if (is_empty(instance))
    {
        return NULL;
    }

    void * data_stream = ((void **)instance->data_stream)[instance->front];
    instance->front = (instance->front + 1) % instance->max_size;
    instance->current_size--;
    return data_stream;
    
}

void destroy(queue_t * instance)
{
    free(instance->data_stream);
    free(instance);
}

queue_t * get_unq_queue(){

    static queue_t * queue_instance = NULL;
    if (queue_instance == NULL) {
        queue_instance = create(MAX_QUEUE_SIZE);
    }
    return queue_instance;
}

#endif
#ifndef QUEUE_C
#define QUEUE_C

#include "../include/queue.h"
#include <stdio.h>

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
    
    pthread_mutex_init(&instance->mutex, NULL);

    return instance;

}


bool enqueue(queue_t * instance, void * data_stream)
{
    if ((instance->current_size == instance->max_size) || (data_stream == NULL) || (!pthread_mutex_trylock(&instance->mutex)))
    {
        return false;
    } else {
        instance->rear = (instance->rear + 1 ) % instance->max_size;
        ((void**) instance->data_stream)[instance->rear] = data_stream;
        instance->current_size++;

        ESP_LOGI(QUEUE_TAG, "enqueue succ");
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

// test if queue is global the same
bool test_queue()
{

    queue_t * queue = get_unq_queue();

    e_actions_t action = 2;
    data_stream_t *data = malloc(sizeof(data_stream_t));
    data->command = action;
    data->ttl = 1212;

    data->crc = 0;

    enqueue(queue, data);

    printf("is empty %d\n", is_empty(queue));   

    queue_t * queue1 = get_unq_queue();

    printf("%p\n", &queue1);

    printf("is empty %d\n", is_empty(queue1));

    void * stream = dequeue(queue1);

    auto payload = (data_stream_t *) (stream);

    printf("ttl: %ld", payload->ttl);

    if (queue==queue1)
    {
        return true;
    }

    return false;
}

#endif
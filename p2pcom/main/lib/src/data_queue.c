#ifndef QUEUE_C
#define QUEUE_C

#include "../include/data_queue.h"
#include "../include/send.h"
#include "../include/websocket.h"
#include <stdio.h>

queue_t * create(int max_size)
{
    if (max_size <= 0)
    {
        return NULL;
    }

    queue_t * instance = malloc(sizeof(queue_t));

    instance->max_size = max_size;
    instance->front = instance->current_size = 0;
    instance->rear = instance->max_size - 1;
    instance->data_stream_array = malloc(max_size * sizeof(data_stream_t));
    
    if (pthread_mutex_init(&instance->mutex, NULL) != 0)
    {
        ESP_LOGE(QUEUE_TAG, "Mutex init failed");
        free(instance->data_stream_array);
        free(instance);
        return NULL;
    }

    return instance;

}

bool enqueue(queue_t *instance, data_stream_t * data_stream)
{
    pthread_mutex_lock(&instance->mutex);

    if (instance == NULL || data_stream == NULL)
        return false;

    bool success = false;

    if (instance->current_size < instance->max_size)
    {
        instance->rear = (instance->rear + 1) % instance->max_size;

        instance->data_stream_array[instance->rear] = *data_stream;

        instance->current_size++;
        success = true;
    }
    else
    {
        ESP_LOGE(QUEUE_TAG, "Queue is full, cannot enqueue");
        success = false;
    }
    pthread_mutex_unlock(&instance->mutex);

    return success;
}

bool is_empty(queue_t * instance)
{
    pthread_mutex_lock(&instance->mutex);
    bool empty = (instance->current_size == 0);
    pthread_mutex_unlock(&instance->mutex);
    return empty;
}

data_stream_t dequeue(queue_t * instance)
{

    pthread_mutex_lock(&instance->mutex);

    data_stream_t data_stream = instance->data_stream_array[instance->front];

    instance->front = (instance->front + 1) % instance->max_size;
    instance->current_size--;

    pthread_mutex_unlock(&instance->mutex);

    return data_stream;
}

void destroy(queue_t * instance)
{
    free(instance->data_stream_array);
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

    data_stream_t stream = dequeue(queue1);

    printf("ttl: %ld", stream.ttl);

    if (queue==queue1)
    {
        return true;
    }

    return false;
}

void do_queue(void *pvParameters)
{
    queue_t * queue = get_unq_queue();

    while (1)
    {
        if (!is_empty(queue))
        {
            data_stream_t stream = dequeue(queue);
            
            #if SWITCH == 0
            if (stream.action == REQUEST || stream.action == HOPPING)
            {
                ESP_ERROR_CHECK(transmit(stream));
            }
            #endif


            #if SWITCH == 1

                if (stream.action == REQUEST)
                {
                    engine_t *engine = get_unq_engine();

                    perform_engine_action(engine, stream.command);

                }

                if (stream.action == RESPONSE)
                {   
                    waiting_for_recv = false;
                    ESP_ERROR_CHECK(transmit(stream));
                }
            #endif

            
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void print_info(void *pvParameters)
{
    queue_t * queue = get_unq_queue();

    while (1)
    {
        ESP_LOGI(QUEUE_TAG, "current queue size: %d", queue->current_size);
        ESP_LOGI(QUEUE_TAG, "global seq: %d", global_seq);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif
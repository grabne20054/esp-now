#ifndef DATA_H
#define DATA_H

#include <stdint.h>
#include <time.h>
#include <pthread.h>

typedef enum engine_status{
    READY=0,
    BUSY=1,
    BLOCKED=2, // Object blocks engine
    ERROR=3
} e_status_t;

typedef enum engine_actions{
    OPEN=0,
    CLOSE=1,
    GETSTATUS=2,
    GETPOSITION=3

}e_actions_t;

typedef struct engine{
    uint16_t position;
    e_status_t status;

    pthread_mutex_t * mutex;

} engine_t;

typedef struct __attribute__((packed)) // necessary for crc
{
    uint32_t seq;
    e_actions_t command;
    uint8_t dest[6];
    time_t sent;
    uint32_t ttl;
    uint32_t crc;

    engine_t * engine;

} data_stream_t;

/*
should only be initialized once per node (static)
*/
typedef struct queue {

    size_t front, rear, max_size, current_size;
    void * data_stream;

    pthread_mutex_t * mutex;


} queue_t;

#endif
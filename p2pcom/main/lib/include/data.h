#include <stdint.h>
#include <time.h>


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

} engine_t;

typedef struct data_stream
{
    e_actions_t command;
    uint8_t dest[6];
    time_t sent;
    uint32_t ttl;
    char *crc;

}data_stream_t;
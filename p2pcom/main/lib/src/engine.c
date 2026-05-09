#ifndef ENGINE_C
#define ENGINE_C

#include "../include/engine.h"

uint16_t get_position(engine_t *engine){
    return engine->position;
}

e_status_t get_engine_status(engine_t *engine){
    return engine->status;
}

#endif

#include "tools.h"

uint32_t get_time_BLEM () {
    time_t receivedAt;
    time (&receivedAt);
    return (uint32_t)(receivedAt);
}


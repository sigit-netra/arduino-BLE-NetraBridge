#pragma once
#include "time.h"


#define ASSERT(x)                                                \
    if (!x) {                                                    \
        printf ("assert failure %s:%d\r\n", __FILE__, __LINE__); \
        while (true)                                             \
            ;                                                    \
    }

constexpr const char* file_name (const char* path) {
    const char* file = path;
    while (*path) {
        if (*path++ == '/') {
            file = path;
        }
    }
    return file;
}

#define BLEM_LOG_ALL(...)                                                          \
    do {                                                                           \
        char fileLine[100] = "";                                                   \
        char ARG[100]      = "";                                                   \
        printf ("[%lu:%s:%d] ", get_time_BLEM (), file_name (__FILE__), __LINE__); \
        printf (__VA_ARGS__);                                                      \
        printf ("\r\n");                                                           \
    } while (0)

#define BLEM_LOG_SERIAL(...)                                            \
    printf ("[%lu:%s:%d] ", millis (), file_name (__FILE__), __LINE__); \
    printf (__VA_ARGS__);                                               \
    printf ("\r\n");

#include <cstdint>

uint32_t get_time_BLEM ();
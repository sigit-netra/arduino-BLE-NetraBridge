#pragma once

#include "string"
#include <integers.h>
#include <vector>


#define MAX_AES256_SIZE 100

// #define  __attribute__ (())

struct BLE_components {
    std::string BLE_server;
    uint8_t key_AES256[MAX_AES256_SIZE];
};

struct msg_sos {
    uint8_t value[20];
};

struct msg_cancel_sos {
    uint8_t value[20];
};

struct msg_tamper {
    uint8_t value[20];
};

struct msg_process {
    uint8_t msg_process[1] = { 0x01 };
};

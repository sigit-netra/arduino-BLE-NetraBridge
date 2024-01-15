/**
 * @file status.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-09
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once
#include "string"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>


class Status {
  public:
    Status ();

    static Status* GetInstance ();

    uint8_t GetTamperStatus ();
    void SetTamperStatus (uint8_t tamper_status);

    uint8_t GetSOSStatus ();
    void SetSOSStatus (uint8_t sos_status);

    uint8_t GetBLEStatus ();
    void SetBLEStatus (uint8_t BLE_status);

    std::string get_BLE_server ();
    void set_BLE_server (std::string BLE_server);

    uint8_t GetIntervalStatus ();
    void SetIntervalStatus (uint8_t interval_status);


  private:
    static Status* instance;
    SemaphoreHandle_t lock;

    uint8_t _tamper_status   = 0;
    uint8_t _sos_status      = 0;
    uint8_t _BLE_status      = 0;
    uint8_t _interval_status = 0;


    uint8_t _aes256[35] = { 0 };
    std::string _BLE_server;
};

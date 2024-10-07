#pragma once

#include "Arduino.h"
#include "BLEDevice.h"
#include "deviceStatus/deviceStatus.h"
#include "AES_256.h"
#include "tools/tools.h"
#include "taskHandler.h"


class BLE_handler : taskHandler {
  private:
    AES_256 aes256;

  public:
    BLE_handler (/* args */);
    void entrypoint ();
    void entrypoint_parser ();
    bool initialize ();
    void start ();
    void send_eng_msg ();
    void send_ping_location ();
    void set_interval_30m ();
    void set_interval_60m ();


    static void HandlerWrapper (void* _this) {
        static_cast<BLE_handler*> (_this)->entrypoint ();
    }

    static void HandlerWrapper_parser (void* _this) {
        static_cast<BLE_handler*> (_this)->entrypoint_parser ();
    }
};

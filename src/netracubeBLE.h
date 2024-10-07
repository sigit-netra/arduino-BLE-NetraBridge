#pragma once

#include "AT.h"
#include "Arduino.h"
#include "deviceStatus/deviceStatus.h"
#include "esp_task_wdt.h"

// #include "status.hpp"

#define I_BUTTON_SOS (gpio_num_t)0


class netracubeBLE {
  private:
    AT _at;
    // Status* status;

  public:
    void task_init ();
    void task_at_commands ();
    void task_button ();


    static void task_at_commands_wrapper (void* _this) {
        static_cast<netracubeBLE*> (_this)->task_at_commands ();
    }

    static void task_button_wrapper (void* _this) {
        static_cast<netracubeBLE*> (_this)->task_button ();
    }
};
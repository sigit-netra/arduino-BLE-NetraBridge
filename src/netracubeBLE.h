#pragma once

#include "Arduino.h"
#include "AT.h"
#include "status.hpp"

class netracubeBLE {
  private:
    AT _at;
    Status* status;
  public:
    void task_init ();
    void task_at_commands ();


    static void task_at_commands_wrapper (void* _this) {
        static_cast<netracubeBLE*> (_this)->task_at_commands ();
    }
};
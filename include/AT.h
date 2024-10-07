#pragma once

#include "AES_256.h"
#include "Arduino.h"
#include "deviceStatus/deviceStatus.h"
#include "global.hpp"
#include "status.hpp"
#include <ArduinoJson.h>
#include <cstring>
#include <map>
#include <sstream>
#include <string>


class AT {
  public:
    AT ();
    std::string processCommand (std::string input);

  private:
    Status* status;
    deviceStatus* _status;
    AES_256 aes;
};

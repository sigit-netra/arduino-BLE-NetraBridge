#pragma once

#include "status.hpp"
#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include "global.hpp"
#include "Arduino.h"
#include <ArduinoJson.h>
#include "AES_256.h"

class AT {
  public:
    AT ();
    std::string processCommand (std::string input);

  private:
    Status* status;
    AES_256 aes;
};

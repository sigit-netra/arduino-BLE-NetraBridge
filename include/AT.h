#pragma once

#include "status.hpp"
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
};

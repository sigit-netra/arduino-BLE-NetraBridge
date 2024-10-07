#pragma once

#include "stdint.h"
#include <stddef.h>


class storageInterface {
  private:
    /* data */
  public:
    virtual void init ()                 = 0;
    // virtual void open (const char* path) = 0;
    // virtual int close (void* fp)         = 0;
    // virtual size_t read (void* ptr, size_t size, size_t nmemb, void* stream) = 0;
    // virtual size_t write (const void* ptr, size_t size, size_t nmemb, void* stream) = 0;
    // virtual size_t size (void* stream)          = 0;
    // virtual bool remove_file (const char* path) = 0;
};
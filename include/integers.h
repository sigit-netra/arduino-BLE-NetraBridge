#pragma once

// This file specifies definition for custom integer data types.
// 24-bit and 16-bit little-endian unsigned integer.
// Must be compiled with GCC version >= 8.4.0
#include <cstdint>
#include <endian.h>

// We're familiar with something like uint16_t, uint32_t.
// We need 24-bit version of it. And we have to create our own.
// So be it: le_uint24_t is a 24-bit unsigned integer with little-endian type.
struct __attribute__ ((packed)) le_uint24_t {
    le_uint24_t () { data = 0; }

    le_uint24_t (uint32_t value) {
        // Force integer value into big-endian
        data = (value & 0xFF0000) >> 16;
        data |= (value & 0x00FF00);
        data |= (value & 0x0000FF) << 16;
    }

    friend bool operator== (const le_uint24_t& lhs, const le_uint24_t& rhs) {
        return (lhs.data == rhs.data);
    }

    unsigned int data : 24;
};

struct __attribute__ ((packed)) le_uint16_t {
    le_uint16_t () { data = 0; }

    le_uint16_t (uint32_t value) {
// Force integer value into big-endian
// data = __bswap16 (value);
#ifdef __linux__
        data = __bswap_16 (value);
#else
        data = __bswap16 (value);
#endif
    }

    friend bool operator== (const le_uint16_t& lhs, const le_uint16_t& rhs) {
        return (lhs.data == rhs.data);
    }

    uint16_t data;
};
#pragma once

#include <stdint.h>
#include <stddef.h>

// Mock of rweather/Crypto's Crypto.h for native testing
inline bool secure_compare(const void* data1, const void* data2, size_t len) {
  const uint8_t* a = static_cast<const uint8_t*>(data1);
  const uint8_t* b = static_cast<const uint8_t*>(data2);
  uint8_t diff = 0;
  while (len-- > 0) diff |= *a++ ^ *b++;
  return diff == 0;
}

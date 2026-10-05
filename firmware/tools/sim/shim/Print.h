#pragma once
#include <stddef.h>
#include <stdint.h>
class Print {
public:
  virtual ~Print() {}
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) write(b[i]); return n; }
  size_t write(const char *s) { size_t n = 0; while (*s) n += write((uint8_t)*s++); return n; }
  size_t print(const char *s) { return write(s); }
  size_t println(const char *s) { return write(s) + write((uint8_t)'\n'); }
};

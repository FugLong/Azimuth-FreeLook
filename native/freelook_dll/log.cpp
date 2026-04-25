#include "freelook/log.hpp"

#include <Windows.h>

#include <cstdarg>
#include <cstdio>

namespace freelook {

void logf(const char* fmt, ...) noexcept {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  (void)std::vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  buf[sizeof(buf) - 1] = '\0';
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");
}

}  // namespace freelook

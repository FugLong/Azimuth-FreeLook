#include "freelook/pattern.hpp"

#include <cctype>
#include <cstring>

namespace freelook::pattern {

bool parse_ida(const char* ida, std::uint8_t* bytes, std::uint8_t* mask, std::size_t max_len, std::size_t& out_len) noexcept {
  out_len = 0;
  if (!ida || !bytes || !mask || max_len == 0) {
    return false;
  }

  const char* p = ida;
  while (*p != '\0') {
    while (*p == ' ' || *p == '\t') {
      ++p;
    }
    if (*p == '\0') {
      break;
    }

    if (out_len >= max_len) {
      return false;
    }

    if (p[0] == '?' && (p[1] == '?' || p[1] == '\0' || p[1] == ' ' || p[1] == '\t')) {
      bytes[out_len] = 0;
      mask[out_len] = 0;
      ++out_len;
      p += (p[1] == '?') ? 2 : 1;
      continue;
    }

    if (!std::isxdigit(static_cast<unsigned char>(p[0])) || !std::isxdigit(static_cast<unsigned char>(p[1]))) {
      return false;
    }

    auto hex = [](char c) -> int {
      if (c >= '0' && c <= '9') {
        return c - '0';
      }
      if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
      }
      if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
      }
      return -1;
    };

    const int hi = hex(p[0]);
    const int lo = hex(p[1]);
    if (hi < 0 || lo < 0) {
      return false;
    }

    bytes[out_len] = static_cast<std::uint8_t>((hi << 4) | lo);
    mask[out_len] = 0xFF;
    ++out_len;
    p += 2;
  }

  return out_len != 0;
}

const std::uint8_t* find_first(const std::uint8_t* haystack, std::size_t haystack_len, const std::uint8_t* pat, const std::uint8_t* mask,
    std::size_t pat_len) noexcept {
  if (!haystack || haystack_len < pat_len || !pat || !mask || pat_len == 0) {
    return nullptr;
  }

  const std::size_t last = haystack_len - pat_len;
  for (std::size_t i = 0; i <= last; ++i) {
    bool ok = true;
    for (std::size_t j = 0; j < pat_len; ++j) {
      if ((mask[j] & (haystack[i + j] ^ pat[j])) != 0) {
        ok = false;
        break;
      }
    }
    if (ok) {
      return haystack + i;
    }
  }
  return nullptr;
}

std::uint8_t* resolve_mov_abs(const std::uint8_t* insn) noexcept {
  if (!insn) {
    return nullptr;
  }
  // MOV r64, m64: REX.W 8B /r with ModR/M describing [RIP+disp32] in 64-bit mode.
  if (insn[0] != 0x48 || insn[1] != 0x8B) {
    return nullptr;
  }
  if ((insn[2] & 0xC7) != 0x05) {
    return nullptr;
  }

  const std::int32_t disp = *reinterpret_cast<const std::int32_t*>(insn + 3);
  const std::uint8_t* next = insn + 7;
  return const_cast<std::uint8_t*>(next + disp);
}

}  // namespace freelook::pattern

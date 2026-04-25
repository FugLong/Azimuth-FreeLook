#include "freelook/ue_discovery.hpp"

#include "freelook/log.hpp"
#include "freelook/pattern.hpp"
#include "freelook/pe_utils.hpp"

#include <cstring>

namespace freelook::ue {

namespace {

bool is_printable_ascii(char c) noexcept {
  const unsigned char u = static_cast<unsigned char>(c);
  return u >= 0x20 && u < 0x7F;
}

bool vtable_looks_executable(const void* object) noexcept {
  if (!object) {
    return false;
  }

  const auto addr = reinterpret_cast<std::uintptr_t>(object);
  if (addr < 0x10000) {
    return false;
  }

  const void* const vtable = *reinterpret_cast<const void* const*>(object);
  if (!vtable) {
    return false;
  }

  MEMORY_BASIC_INFORMATION mbi{};
  if (VirtualQuery(vtable, &mbi, sizeof(mbi)) == 0) {
    return false;
  }

  const DWORD prot = mbi.Protect & 0xFF;
  return prot == PAGE_EXECUTE || prot == PAGE_EXECUTE_READ || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_EXECUTE_WRITECOPY;
}

bool try_each_gworld_pattern(const std::uint8_t* text, std::size_t text_len, void*& out_world) noexcept {
  static const char* kPatterns[] = {
      // Common "load UWorld pointer from RIP-relative global" shape seen across many UE4/UE5 shipping builds.
      "48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74",
      // Alternate register / tail differs, still ends up dereferencing a global UWorld*.
      "48 8B 1D ? ? ? ? 48 85 DB 74 ? 41 B0 01",
      // Another frequently seen variant (still MOV r64, [RIP+disp32] prologue).
      "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B D8",
  };

  std::uint8_t bytes[64]{};
  std::uint8_t mask[64]{};

  for (const char* ida : kPatterns) {
    std::size_t pat_len = 0;
    if (!pattern::parse_ida(ida, bytes, mask, sizeof(bytes), pat_len)) {
      continue;
    }

    const std::uint8_t* cursor = text;
    const std::uint8_t* const end = text + text_len;
    while (cursor < end) {
      const std::size_t remain = static_cast<std::size_t>(end - cursor);
      if (remain < pat_len) {
        break;
      }

      const std::uint8_t* hit = pattern::find_first(cursor, remain, bytes, mask, pat_len);
      if (!hit) {
        break;
      }

      std::uint8_t* storage = pattern::resolve_mov_abs(hit);
      if (!storage) {
        cursor = hit + 1;
        continue;
      }

      void* world = *reinterpret_cast<void**>(storage);
      if (vtable_looks_executable(world)) {
        out_world = world;
        return true;
      }

      cursor = hit + 1;
    }
  }

  out_world = nullptr;
  return false;
}

}  // namespace

bool sniff_embedded_build_label(HMODULE engine, char* out, std::size_t out_sz) noexcept {
  if (!engine || !out || out_sz == 0) {
    return false;
  }
  out[0] = '\0';

  const std::uint8_t* rdata = nullptr;
  std::size_t rdata_size = 0;
  if (!pe::get_section_range(engine, ".rdata", rdata, rdata_size)) {
    return false;
  }

  // Epic embeds readable build flavor strings like: ++UE5+Release-5.3-CL-12345678
  for (std::size_t i = 0; i + 4 < rdata_size; ++i) {
    if (rdata[i] != '+' || rdata[i + 1] != '+' || rdata[i + 2] != 'U' || rdata[i + 3] != 'E') {
      continue;
    }

    std::size_t j = i;
    std::size_t k = 0;
    for (; j < rdata_size && k + 1 < out_sz; ++j) {
      const char c = static_cast<char>(rdata[j]);
      if (!is_printable_ascii(c)) {
        break;
      }
      out[k++] = c;
    }
    out[k] = '\0';
    return k != 0;
  }

  return false;
}

bool try_resolve_gworld(HMODULE engine, void*& out_world) noexcept {
  out_world = nullptr;

  const std::uint8_t* text = nullptr;
  std::size_t text_len = 0;
  if (!pe::get_section_range(engine, ".text", text, text_len)) {
    static bool logged = false;
    if (!logged) {
      freelook::logf("[azimuth_freelook] ue: failed to locate .text in engine module");
      logged = true;
    }
    return false;
  }

  return try_each_gworld_pattern(text, text_len, out_world);
}

}  // namespace freelook::ue

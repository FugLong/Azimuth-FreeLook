#include "freelook/pe_utils.hpp"

#include <cstring>

namespace freelook::pe {

bool get_section_range(HMODULE module, const char* section_name, const std::uint8_t*& out_start, std::size_t& out_size) noexcept {
  if (!module || !section_name) {
    return false;
  }

  auto* base = reinterpret_cast<const std::uint8_t*>(module);
  auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
  if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
    return false;
  }

  auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
  if (nt->Signature != IMAGE_NT_SIGNATURE) {
    return false;
  }

  const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
  const WORD n = nt->FileHeader.NumberOfSections;
  for (WORD i = 0; i < n; ++i) {
    if (std::memcmp(sec[i].Name, section_name, IMAGE_SIZEOF_SHORT_NAME) == 0) {
      out_start = base + sec[i].VirtualAddress;
      out_size = static_cast<std::size_t>(sec[i].Misc.VirtualSize);
      return out_size != 0;
    }
  }

  return false;
}

}  // namespace freelook::pe

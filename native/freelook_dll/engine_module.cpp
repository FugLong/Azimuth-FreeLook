#include "freelook/engine_module.hpp"

#include <Psapi.h>

#include <cwchar>

namespace freelook::engine {

namespace {

bool basename_matches_engine(const wchar_t* full_path) noexcept {
  const wchar_t* slash = wcsrchr(full_path, L'\\');
  const wchar_t* base = slash ? (slash + 1) : full_path;

  // Typical Epic-shipped layout: UnrealEngine-Win64-Shipping.dll
  if (!wcsstr(base, L"UnrealEngine")) {
    return false;
  }
  if (!wcsstr(base, L"Win64")) {
    return false;
  }
  // Avoid picking up unrelated tools; shipping/test/debug game binaries are what we want.
  return wcsstr(base, L"Shipping") != nullptr || wcsstr(base, L"Test") != nullptr || wcsstr(base, L"DebugGame") != nullptr ||
         wcsstr(base, L"Debug") != nullptr;
}

}  // namespace

bool find_main_engine_module(HMODULE& out_module, wchar_t* path_out, std::size_t path_cch) noexcept {
  out_module = nullptr;
  if (!path_out || path_cch == 0) {
    return false;
  }
  path_out[0] = L'\0';

  HMODULE mods[512]{};
  DWORD needed = 0;
  if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &needed)) {
    return false;
  }

  const DWORD count = needed / static_cast<DWORD>(sizeof(HMODULE));
  for (DWORD i = 0; i < count; ++i) {
    wchar_t path[MAX_PATH]{};
    const DWORD n = GetModuleFileNameW(mods[i], path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
      continue;
    }
    if (!basename_matches_engine(path)) {
      continue;
    }

    out_module = mods[i];
    wcsncpy_s(path_out, path_cch, path, _TRUNCATE);
    return true;
  }

  return false;
}

}  // namespace freelook::engine

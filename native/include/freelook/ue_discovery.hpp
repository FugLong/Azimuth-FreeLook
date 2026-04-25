#pragma once

#include <Windows.h>

namespace freelook::ue {

// Best-effort: read an embedded "++UE5+Release-..." style string from the engine module.
bool sniff_embedded_build_label(HMODULE engine, char* out, std::size_t out_sz) noexcept;

// Try a small set of common GWorld access patterns in .text, resolve RIP-relative storage, validate UObject vtable range.
// Returns true if a plausible non-null UWorld pointer was found (still game/engine specific).
bool try_resolve_gworld(HMODULE engine, void*& out_world) noexcept;

}  // namespace freelook::ue

#pragma once

#include <Windows.h>

namespace freelook::engine {

// Locates the main Unreal Engine module in the current process (e.g. UnrealEngine-Win64-Shipping.dll).
bool find_main_engine_module(HMODULE& out_module, wchar_t* path_out, std::size_t path_cch) noexcept;

}  // namespace freelook::engine

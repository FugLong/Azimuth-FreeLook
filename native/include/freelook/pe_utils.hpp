#pragma once

#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace freelook::pe {

bool get_section_range(HMODULE module, const char* section_name, const std::uint8_t*& out_start, std::size_t& out_size) noexcept;

}  // namespace freelook::pe

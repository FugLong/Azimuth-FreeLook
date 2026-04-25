#pragma once

#include <cstddef>
#include <cstdint>

namespace freelook::pattern {

// IDA-style pattern: hex bytes and '?' wildcards, space-separated (e.g. "48 8B 05 ? ? ? ?").
bool parse_ida(const char* ida, std::uint8_t* bytes, std::uint8_t* mask, std::size_t max_len, std::size_t& out_len) noexcept;

const std::uint8_t* find_first(const std::uint8_t* haystack, std::size_t haystack_len, const std::uint8_t* pat, const std::uint8_t* mask,
    std::size_t pat_len) noexcept;

// x64 RIP-relative addressing for 7-byte "MOV r64, [RIP+disp32]" (opcode 0x48 0x8B 0x0? where modrm is 0x05/0x0D/0x15/0x1D/0x25/0x2D/0x35/0x3D).
std::uint8_t* resolve_mov_abs(const std::uint8_t* insn) noexcept;

}  // namespace freelook::pattern

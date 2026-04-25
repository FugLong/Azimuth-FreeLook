#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace freelook::opentrack_udp {

// OpenTrack "UDP over network" output: six little-endian doubles (Windows/OpenTrack default).
// https://opentrack-opentrack.mintlify.app/protocols/udp
inline constexpr std::size_t kPacketBytes = 48;

#pragma pack(push, 1)
struct PosePacket {
  double yaw_deg;
  double pitch_deg;
  double roll_deg;
  double x_cm;
  double y_cm;
  double z_cm;
};
#pragma pack(pop)

static_assert(sizeof(PosePacket) == kPacketBytes, "Unexpected PosePacket layout");

inline bool try_unpack(const std::uint8_t* data, std::size_t len, PosePacket& out) {
  if (len != kPacketBytes) {
    return false;
  }
  std::memcpy(&out, data, kPacketBytes);
  return true;
}

}  // namespace freelook::opentrack_udp

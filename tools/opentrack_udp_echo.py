#!/usr/bin/env python3
"""
Decode OpenTrack "UDP over network" packets (6 x float64 = 48 bytes).
Handy on macOS/Linux while the game injector is built and tested on Windows.

Configure OpenTrack: Output -> UDP over network -> destination IP/port matching this script.
"""

from __future__ import annotations

import argparse
import socket
import struct


def main() -> int:
  parser = argparse.ArgumentParser(description="Print OpenTrack UDP pose packets.")
  parser.add_argument("--bind", default="0.0.0.0", help="Listen address")
  parser.add_argument("--port", type=int, default=4242, help="UDP port")
  args = parser.parse_args()

  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  sock.bind((args.bind, args.port))
  print(f"Listening on udp://{args.bind}:{args.port} (expecting 48-byte packets)")

  while True:
    data, addr = sock.recvfrom(4096)
    if len(data) != 48:
      print(f"{addr}: ignored packet len={len(data)}")
      continue
    yaw, pitch, roll, x_cm, y_cm, z_cm = struct.unpack("<dddddd", data)
    print(
      f"{addr}: yaw={yaw:+.2f} pitch={pitch:+.2f} roll={roll:+.2f} "
      f"cm=({x_cm:+.1f},{y_cm:+.1f},{z_cm:+.1f})"
    )


if __name__ == "__main__":
  raise SystemExit(main())

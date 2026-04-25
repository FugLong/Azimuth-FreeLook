# Azimuth FreeLook

Experimental **OpenTrack-driven freelook** for Unreal Engine games on **Windows**, designed to pair with **Azimuth** head-tracking hardware (and anything else that can drive OpenTrack).

Native code targets **MSVC x64** (injected DLL + small injector). macOS can still be used for **editing** and for validating **OpenTrack UDP** with `tools/opentrack_udp_echo.py`.

## Repository layout

| Path | Role |
|------|------|
| `native/injector` | `azimuth_injector.exe` — loads `azimuth_freelook.dll` into a target process by PID (scaffold). |
| `native/freelook_dll` | `azimuth_freelook.dll` — in-process module (UE hooks not implemented yet). |
| `native/include/freelook/opentrack_udp.hpp` | Shared constants/layout for OpenTrack’s 48-byte UDP pose packet. |
| `tools/opentrack_udp_echo.py` | Cross-platform UDP listener for quick OpenTrack sanity checks. |

## Build (Windows)

Requirements: **Visual Studio 2022** (Desktop development with C++), **CMake 3.20+**.

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Artifacts with the **Visual Studio** generator (typical):

- `build/bin/Release/azimuth_injector.exe`
- `build/bin/Release/azimuth_freelook.dll`

Use **x64** only; match the architecture of the game process.

## OpenTrack UDP test (macOS or anywhere)

1. Run: `python3 tools/opentrack_udp_echo.py --port 4242`
2. In OpenTrack: **Output → UDP over network**, set the IP of this machine and port **4242**.

You should see yaw/pitch/roll and translation values as you move the tracker.

## Injector usage (Windows, experimental)

```bat
azimuth_injector.exe C:\full\path\to\azimuth_freelook.dll <PID>
```

`PID` is the decimal process ID of the target game. Injecting into software you do not own or into **online / anti-cheat protected** titles may violate terms of service or trigger bans. Intended for **offline / single-player** experimentation only.

## macOS native support

Not planned for the first iterations: few UE titles ship for Apple silicon, and injected Windows DLLs are the compatibility sweet spot.

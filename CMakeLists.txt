cmake_minimum_required(VERSION 3.20)
project(azimuth-freelook VERSION 0.1.0 LANGUAGES CXX)

if(WIN32)
  add_subdirectory(native)
else()
  message(
    STATUS
    "Skipping native Windows targets on this platform. "
    "Build on Windows with MSVC for the injector and DLL; "
    "use tools/opentrack_udp_echo.py here to validate OpenTrack UDP output.")
endif()

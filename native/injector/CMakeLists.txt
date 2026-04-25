add_executable(azimuth_injector main.cpp)
target_compile_features(azimuth_injector PRIVATE cxx_std_17)
target_compile_definitions(azimuth_injector PRIVATE UNICODE _UNICODE)

if(MSVC)
  target_compile_options(azimuth_injector PRIVATE /W4 /permissive-)
endif()

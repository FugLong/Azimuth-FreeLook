add_library(
  azimuth_freelook
  SHARED
  dllmain.cpp
  freelook_main.cpp
  log.cpp
  pe_utils.cpp
  pattern.cpp
  engine_module.cpp
  ue_discovery.cpp)

target_compile_features(azimuth_freelook PRIVATE cxx_std_17)
target_include_directories(azimuth_freelook PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_link_libraries(azimuth_freelook PRIVATE Psapi)

if(MSVC)
  target_compile_options(azimuth_freelook PRIVATE /W4 /permissive-)
endif()

set_target_properties(azimuth_freelook PROPERTIES OUTPUT_NAME "azimuth_freelook")

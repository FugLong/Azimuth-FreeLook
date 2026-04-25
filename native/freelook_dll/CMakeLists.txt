add_library(azimuth_freelook SHARED dllmain.cpp)
target_compile_features(azimuth_freelook PRIVATE cxx_std_17)
target_include_directories(azimuth_freelook PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")

if(MSVC)
  target_compile_options(azimuth_freelook PRIVATE /W4 /permissive-)
endif()

set_target_properties(azimuth_freelook PROPERTIES OUTPUT_NAME "azimuth_freelook")

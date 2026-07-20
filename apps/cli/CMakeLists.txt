add_executable(canopy-cli
  src/main.cpp)

target_link_libraries(canopy-cli
  PRIVATE canopy::export canopy_warnings)

add_library(canopy_foundation STATIC
  src/diagnostics.cpp
  src/hash.cpp
  src/ids.cpp
  src/json.cpp
  src/random.cpp
  src/curve.cpp)

target_include_directories(canopy_foundation PUBLIC include)
target_link_libraries(canopy_foundation PRIVATE canopy_warnings)
add_library(canopy::foundation ALIAS canopy_foundation)

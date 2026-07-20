add_library(canopy_runtime_forest STATIC
  src/forest.cpp)

target_include_directories(canopy_runtime_forest PUBLIC include)
target_link_libraries(canopy_runtime_forest
  PUBLIC canopy::runtime_core
  PRIVATE canopy_warnings)
add_library(canopy::runtime_forest ALIAS canopy_runtime_forest)

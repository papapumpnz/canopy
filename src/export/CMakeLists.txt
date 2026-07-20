add_library(canopy_export STATIC
  src/obj_export.cpp)

target_include_directories(canopy_export PUBLIC include)
target_link_libraries(canopy_export
  PUBLIC canopy::evaluation
  PRIVATE canopy_warnings)
add_library(canopy::export ALIAS canopy_export)

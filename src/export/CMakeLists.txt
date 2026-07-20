add_library(canopy_export STATIC
  src/obj_export.cpp
  src/gltf_export.cpp
  src/mesh_merge.cpp
  src/rt_compile.cpp)

target_include_directories(canopy_export PUBLIC include)
target_link_libraries(canopy_export
  PUBLIC canopy::evaluation canopy::runtime_core
  PRIVATE canopy_warnings)
add_library(canopy::export ALIAS canopy_export)

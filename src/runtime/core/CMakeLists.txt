# Runtime loader: depends on canopy_foundation ONLY (never authoring libs).
add_library(canopy_runtime_core STATIC
  src/loader.cpp)

target_include_directories(canopy_runtime_core PUBLIC include)
target_link_libraries(canopy_runtime_core
  PUBLIC canopy::foundation
  PRIVATE canopy_warnings)
add_library(canopy::runtime_core ALIAS canopy_runtime_core)

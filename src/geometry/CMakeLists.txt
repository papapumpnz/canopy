add_library(canopy_geometry STATIC
  src/spline.cpp
  src/frames.cpp
  src/sweep.cpp
  src/polygon.cpp)

target_include_directories(canopy_geometry PUBLIC include)
target_link_libraries(canopy_geometry
  PUBLIC canopy::foundation
  PRIVATE canopy_warnings)
add_library(canopy::geometry ALIAS canopy_geometry)

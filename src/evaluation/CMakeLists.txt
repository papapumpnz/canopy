add_library(canopy_evaluation STATIC
  src/evaluate.cpp)

target_include_directories(canopy_evaluation PUBLIC include)
target_link_libraries(canopy_evaluation
  PUBLIC canopy::document canopy::geometry
  PRIVATE canopy_warnings)
add_library(canopy::evaluation ALIAS canopy_evaluation)

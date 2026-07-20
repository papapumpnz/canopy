add_library(canopy_document STATIC
  src/document.cpp
  src/registry.cpp
  src/project_io.cpp)

target_include_directories(canopy_document PUBLIC include)
target_link_libraries(canopy_document
  PUBLIC canopy::foundation
  PRIVATE canopy_warnings)
add_library(canopy::document ALIAS canopy_document)

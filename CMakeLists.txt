cmake_minimum_required(VERSION 3.25)

project(Canopy
  VERSION 0.1.0
  DESCRIPTION "Project Canopy - clean-room vegetation authoring platform"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(CANOPY_BUILD_GUI "Build the Qt modeler application" OFF)
option(CANOPY_BUILD_TESTS "Build tests" ON)
option(CANOPY_SANITIZE "Enable ASan+UBSan" OFF)

include(cmake/CanopyWarnings.cmake)

if(CANOPY_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

if(CANOPY_BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(src/foundation)
add_subdirectory(src/document)
add_subdirectory(src/geometry)
add_subdirectory(src/evaluation)
add_subdirectory(src/export)
add_subdirectory(apps/cli)

if(CANOPY_BUILD_TESTS)
  add_subdirectory(tests/unit)
  add_subdirectory(tests/integration)
endif()

if(CANOPY_BUILD_GUI)
  message(FATAL_ERROR "The modeler GUI is not implemented yet; configure with -DCANOPY_BUILD_GUI=OFF")
endif()

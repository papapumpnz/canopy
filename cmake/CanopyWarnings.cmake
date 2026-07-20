# Warning policy (backlog B-008): warnings are errors in project code.
add_library(canopy_warnings INTERFACE)

if(MSVC)
  target_compile_options(canopy_warnings INTERFACE /W4 /WX /permissive-)
else()
  target_compile_options(canopy_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC's -Wmaybe-uninitialized has known false positives at -O2 with
    # std::variant-held containers (fires inside libstdc++'s _Rb_tree move,
    # not project code). Real uninitialized reads are caught by the
    # ASan/UBSan presets, which stay mandatory in CI.
    target_compile_options(canopy_warnings INTERFACE -Wno-maybe-uninitialized)
  endif()
endif()

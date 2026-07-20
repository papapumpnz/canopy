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
endif()

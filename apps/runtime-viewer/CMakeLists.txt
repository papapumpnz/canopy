# Headless runtime reference demo: loads .canopyrt models, scatters a
# deterministic forest, culls + selects LODs from a camera, and bakes the
# visible set to OBJ for diagnostic rendering. Uses ONLY runtime libraries —
# living proof the runtime path needs no authoring code.
add_executable(canopy-runtime-demo
  src/main.cpp)

target_link_libraries(canopy-runtime-demo
  PRIVATE canopy::runtime_forest canopy_warnings)

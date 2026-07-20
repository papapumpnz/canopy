# Performance baselines (22_NONFUNCTIONAL_REQUIREMENTS.md: "measured, not
# inferred"). Build with the release preset; debug numbers are meaningless.
add_executable(canopy-bench
  src/bench_canopy.cpp)

target_link_libraries(canopy-bench
  PRIVATE canopy::export canopy::runtime_forest canopy_warnings)

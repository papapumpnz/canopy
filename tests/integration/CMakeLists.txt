add_executable(test_project_io test_project_io.cpp)
target_include_directories(test_project_io PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_project_io PRIVATE canopy::document canopy_warnings)
add_test(NAME test_project_io COMMAND test_project_io)

# Bootstrap integration gate commands (34_BOOTSTRAP_BACKLOG.md) run as tests.
set(FIXTURE ${CMAKE_SOURCE_DIR}/tests/assets/MinimalTrunk.canopyproj)
set(PRESET ${CMAKE_SOURCE_DIR}/tests/presets/obj-diagnostic.json)

add_test(NAME cli_validate COMMAND canopy-cli validate ${FIXTURE} --json)
add_test(NAME cli_evaluate COMMAND canopy-cli evaluate ${FIXTURE} --profile production --json)
add_test(NAME cli_export COMMAND canopy-cli export ${FIXTURE} --preset ${PRESET}
                                 --out ${CMAKE_BINARY_DIR}/exports/minimal-trunk --json)
add_test(NAME cli_export_gltf COMMAND canopy-cli export ${FIXTURE}
                                      --preset ${CMAKE_SOURCE_DIR}/tests/presets/gltf.json
                                      --out ${CMAKE_BINARY_DIR}/exports/minimal-trunk-glb --json)

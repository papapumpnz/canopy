# Repository and build design

## Monorepo layout

```text
/
  CMakeLists.txt
  CMakePresets.json
  vcpkg.json
  AGENTS.md
  LICENSES/
  cmake/
  docs/
  schemas/
    document/
    runtime/
    plugin/
  src/
    base/
    document/
    graph/
    eval/
    geometry/
    material/
    animation/
    lod/
    export/
    runtime_compiler/
    runtime/
    viewport/
    modeler/
    cli/
  plugins/
    import_obj/
    export_obj/
    export_gltf/
    export_usd/
    export_alembic/
    export_fbx_optional/
    engine_unity/
    engine_unreal/
    engine_godot/
  bindings/
    python/
    c_api/
  shaders/
    common/
    viewport/
    runtime_reference/
    unity/
    unreal/
    godot/
  tests/
    unit/
    property/
    integration/
    golden_geometry/
    golden_images/
    fuzz/
    performance/
    compatibility/
  samples/
    documents/
    assets/
    rules/
    packing/
    integrations/
  tools/
    schema_codegen/
    asset_linter/
    golden_update/
    license_report/
  third_party/
    README.md
```

## Build system

- CMake is authoritative for C++ targets.
- CMake presets define developer, CI, sanitizer, coverage, shipping, and plugin-SDK configurations.
- A manifest-based dependency manager locks source revisions and binary provenance.
- Rust is used only to build `wgpu-native` unless a later ADR approves additional Rust modules.
- Generated schemas and shader artifacts must be reproducible from checked-in source files.
- No generated binary is committed unless it is required for bootstrapping and has a documented regeneration path.

## Build target groups

### Core targets

- `canopy_base`
- `canopy_document`
- `canopy_graph`
- `canopy_eval`
- `canopy_geometry`
- `canopy_material`
- `canopy_animation`
- `canopy_lod`
- `canopy_export`
- `canopy_runtime_compiler`

### Applications

- `canopy-modeler`
- `canopy-cli`
- `canopy-worker`
- `canopy-runtime-reference`

### SDKs

- `canopy_core_sdk`
- `canopy_runtime_c`
- `canopy_runtime_cpp`
- `canopy_plugin_sdk`
- Python wheel

## Compiler policy

- Treat warnings as errors in project code.
- Enable strict conformance, stack protection, control-flow protection where supported, and hidden symbol visibility by default.
- Shipping builds use link-time optimization after deterministic-build tests.
- Sanitizer presets include address, undefined behavior, thread, and memory where supported.
- Runtime SDK headers avoid exceptions and RTTI across the ABI boundary.

## Supported authoring build matrix

| Platform | Compiler | UI | Viewport backend |
|---|---|---|---|
| Windows x64 | Current supported MSVC | Qt 6 Widgets | D3D12 or Vulkan through WebGPU |
| macOS arm64 | Current supported Apple Clang | Qt 6 Widgets | Metal through WebGPU |
| Linux x64 | Current supported Clang and GCC | Qt 6 Widgets | Vulkan through WebGPU |

The runtime SDK additionally supports platform-specific licensed branches. Public code must keep platform adapters behind narrow interfaces so confidential console code can live in separate repositories.

## Dependency policy

Every dependency requires:

- Exact version or commit lock
- Source URL and integrity hash
- License and notice text
- Security maintenance status
- Build options used
- Whether it enters public headers or ABI
- Replacement or vendoring strategy

Preferred categories:

- Qt 6 core GUI modules, dynamically linked under an appropriate license or commercially licensed
- `wgpu-native` and WebGPU headers for viewport rendering
- OpenUSD for USD interchange
- Alembic for `.abc`
- Autodesk FBX SDK only in an optional adapter built by the user or licensed distributor
- OpenImageIO and OpenColorIO for production image and color workflows
- meshoptimizer, xatlas, MikkTSpace, and robust geometry libraries with permissive licenses
- Lua 5.4-compatible interpreter for rules and packing
- FlatBuffers, Cap'n Proto, or an equivalent versioned binary schema tool for runtime data; final choice is an ADR
- Catch2 or GoogleTest plus RapidCheck-like property testing
- libFuzzer/AFL++ integration for parsers

## CI stages

1. Formatting and generated-file consistency
2. License and dependency audit
3. Configure and compile on all authoring platforms
4. Unit and property tests
5. Parser fuzz smoke tests
6. Golden geometry and image tests
7. Export/import integration tests
8. Runtime reference-scene tests
9. Performance threshold tests
10. Packaging, signing, SBOM, and provenance generation

## Artifact policy

Every build artifact includes:

- Semantic version and commit ID
- Schema versions
- Plugin API version
- Compiler and dependency lock identifier
- Build configuration
- Reproducibility/provenance statement
- SPDX SBOM
- Third-party notices

## Branch and review policy

- Main branch is always buildable.
- Feature work is merged behind capability flags when incomplete.
- Each pull request references requirement IDs and test IDs.
- Public API changes require an ADR and compatibility review.
- Golden updates require a visual or geometry-diff report and a second reviewer.

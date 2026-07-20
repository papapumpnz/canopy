# Public API contracts

## API families

Project Canopy exposes distinct APIs for authoring, headless automation, plugins and runtime use. They share value definitions but do not expose private implementation classes.

| API | Stability target | Consumers |
|---|---|---|
| C++ Authoring SDK | Source-compatible within a major release for declared stable headers | Modeler, CLI, trusted tools |
| C Runtime ABI | Binary-compatible within a major ABI | Engines, foreign-language bindings |
| Plugin C ABI | Binary-compatible within a major ABI | Native extensions |
| Python API | Source-compatible within a major release | Pipelines and tests |
| Lua Rules API | Source-compatible with deprecation period | Artist rules |
| CLI | Stable flags, exit codes and JSON schemas | CI and farms |
| File schemas | Versioned with explicit migration | All products |

## Fundamental types

- `Uuid`: stable 128-bit object identity
- `ContentHash`: algorithm identifier plus digest
- `Version`: semantic or schema version as appropriate
- `Status`: code, category, message, source location and diagnostics
- `Expected<T, E>`: result type in C++
- `Span<T>` and `StringView`: non-owning inputs
- `OwnedBuffer`: allocator-tagged ownership across ABI boundaries
- `CancellationToken`: cooperative cancellation
- `ProgressSink`: hierarchical progress events
- `TaskHandle`: asynchronous operation handle
- `PropertyPath`: parsed, validated property identity
- `UnitValue`: numeric value with dimensional type

All API text is UTF-8. Public C functions are exception free. C++ exceptions do not cross module boundaries.

## Authoring document API

```cpp
namespace canopy {

class Document {
public:
    static Expected<Document, OpenError> open(
        const std::filesystem::path& path,
        const OpenOptions& options = {});

    [[nodiscard]] DocumentId id() const noexcept;
    [[nodiscard]] SchemaVersion schema_version() const noexcept;
    [[nodiscard]] Revision revision() const noexcept;

    Transaction begin_transaction(std::string_view label);
    Expected<EvaluationSnapshot, EvaluationError> evaluate(
        const EvaluateOptions& options,
        CancellationToken cancellation = {});

    Expected<void, SaveError> save(const SaveOptions& options = {});
    Expected<void, SaveError> save_as(
        const std::filesystem::path& path,
        const SaveOptions& options = {});
};

}
```

A `Transaction` owns all mutations. It can commit, roll back or create nested savepoints. Mutating an object from a stale revision returns a conflict error rather than writing to an unexpected object.

## Graph API

```cpp
class GeneratorGraph {
public:
    GeneratorView find(GeneratorId id) const;
    std::vector<GeneratorView> query(const GeneratorQuery&) const;
    Expected<GeneratorId, GraphError> create(
        TypeId type, GeneratorId parent, std::string_view name);
    Expected<void, GraphError> link(GeneratorId parent, GeneratorId child);
    Expected<void, GraphError> erase(GeneratorId id, ErasePolicy policy);
};
```

Graph constraints are explicit. A failed link explains cycle, parent-type, cardinality or plugin-availability violations.

## Property API

Properties are reflected through descriptors:

```cpp
struct PropertyDescriptor {
    PropertyId id;
    std::string_view path;
    ValueType value_type;
    UnitDimension unit;
    PropertyFlags flags;
    Value default_value;
    std::optional<NumericRange> range;
    std::span<const EnumItem> enum_items;
};
```

A property value can include a scalar, variance, profile curve, expression, season channel and node override according to descriptor capabilities. Callers cannot attach unsupported components.

## Evaluation API

`EvaluateOptions` specifies:

- Profile name
- Requested products: skeleton, render geometry, collision, animation, diagnostics
- Resolution or LOD preview value
- Season and time
- Determinism mode
- Cache policy
- Maximum worker count
- Memory budget
- Selected subgraph or full document

`EvaluationSnapshot` is immutable, revision tagged and safe for concurrent readers. Geometry arrays use spans whose lifetime is the snapshot.

## Asset API

Assets are identified by URI and content hash. Supported URI schemes include project-relative, package, file where permitted, and registry. An `AssetResolver` is injected; core evaluation does not perform hidden network requests.

## Export API

```cpp
Expected<ExportResult, ExportError> export_snapshot(
    const EvaluationSnapshot& snapshot,
    const ExportPreset& preset,
    IOutputTransaction& output,
    CancellationToken cancellation = {},
    ProgressSink* progress = nullptr);
```

The output transaction provides path validation, temporary writes, checksums and atomic commit. Export results include a machine-readable manifest and all diagnostics.

## Runtime C ABI

Illustrative API:

```c
typedef struct canopy_rt_context canopy_rt_context;
typedef struct canopy_rt_asset canopy_rt_asset;
typedef struct canopy_rt_forest canopy_rt_forest;

typedef struct canopy_rt_api_version {
    uint32_t struct_size;
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    uint16_t reserved;
} canopy_rt_api_version;

typedef struct canopy_rt_context_desc {
    uint32_t struct_size;
    uint32_t flags;
    canopy_allocator allocator;
    canopy_log_callback log_callback;
    void* user_data;
} canopy_rt_context_desc;

CANOPY_RT_API canopy_status canopy_rt_create_context(
    const canopy_rt_context_desc* desc,
    canopy_rt_context** out_context);

CANOPY_RT_API canopy_status canopy_rt_load_asset_from_memory(
    canopy_rt_context* context,
    const void* bytes,
    uint64_t size,
    const canopy_rt_asset_load_desc* desc,
    canopy_rt_asset** out_asset);
```

Every function validates nulls and `struct_size`. Handles are reference counted or follow explicit create/destroy ownership documented per type.

## Forest API

```c
CANOPY_RT_API canopy_status canopy_rt_forest_set_view(
    canopy_rt_forest* forest,
    uint32_t view_index,
    const canopy_rt_view* view);

CANOPY_RT_API canopy_status canopy_rt_forest_cull(
    canopy_rt_forest* forest,
    const canopy_rt_cull_desc* desc,
    canopy_rt_visibility_result* out_result);
```

A visibility result owns or references arrays of visible cells, instances and draw packets according to a declared lifetime ending at the next mutation or explicit release.

## Callback rules

- Callbacks state which thread invokes them.
- No callback occurs under an internal non-reentrant lock.
- Callback user data is opaque.
- Callbacks may return cancellation or failure.
- Logging is rate limited by category.
- APIs do not retain stack-backed input spans after return unless explicitly documented.

## Error taxonomy

Top-level categories:

- Invalid argument
- Invalid state
- Not found
- Unsupported feature
- Schema mismatch
- Missing dependency
- Data corruption
- Resource exhausted
- Permission denied
- Cancelled
- Conflict
- Plugin failure
- I/O failure
- Internal invariant failure

Errors carry nested diagnostics and stable machine codes. Human messages are not parsed by automation.

## Asynchrony

Long-running C++ operations return `Task<T>` or accept caller scheduling. C APIs expose task handles with poll, wait, cancel and completion callback operations. Waiting is never required on the UI thread.

## Allocators

- The host can supply allocation, reallocation, free and aligned-allocation callbacks.
- The module that allocates memory also frees it unless ownership is explicitly transferred through an `OwnedBuffer` destructor function.
- Allocation tags identify geometry, texture, cache, forest, plugin and transient use.
- Out-of-memory paths are tested.

## Reflection and code generation

API declarations, documentation, Python stubs, JSON schemas and ABI layout tests derive from a single interface description where practical. Generated code is checked into release archives and regenerated in CI to detect drift.

## Deprecation policy

- Deprecated symbols are annotated in headers and documentation.
- A replacement and earliest removal major version are stated.
- CLI aliases emit structured warnings.
- File migrations do not depend on deprecated runtime code after a major release boundary.
- ABI symbols are not removed within a major ABI.

## API conformance artifacts

Each release ships:

- Public headers
- API reference Markdown
- C and C++ minimal examples
- Python type stubs
- Lua Rules reference
- CLI JSON Schemas
- ABI layout dump per platform
- Runtime format conformance vectors
- Compatibility and deprecation tables

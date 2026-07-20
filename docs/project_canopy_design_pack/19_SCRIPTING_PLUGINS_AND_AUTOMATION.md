# Scripting, plugins, and automation

## Design goals

Extensibility must cover artist-authored rules, studio automation, custom generators, exporters, data packing, validators and UI tools without making project files arbitrary executable programs.

The system provides four extension tiers:

| Tier | Technology | Primary use | Trust level |
|---|---|---|---|
| Rules | Sandboxed Lua | Artist controls and property relationships | Untrusted by default |
| Automation | Python and CLI | Batch generation, asset processing and pipeline orchestration | Studio trusted |
| Native plugins | C ABI plus optional C++ wrappers | Generators, importers, exporters and render backends | Explicitly trusted |
| Out-of-process tools | JSON-RPC/gRPC-like local protocol | Untrusted or crash-prone heavy extensions | Isolated |

## Rules system

Rules create high-level controls and map them to model properties. A rule document declares controls, dependencies, visibility conditions and callbacks.

Supported controls:

- Float and integer slider
- Boolean
- Enum
- Color
- Curve
- Text and read-only label
- Button/action
- Asset reference
- Generator or node selector
- Group and collapsible section

Rules can:

- Read documented model and generator properties
- Set properties inside a transaction
- Combine values and curves
- Search generators by stable identifier, type, tag or path
- React to a control change
- Request a bounded graph evaluation
- Emit diagnostics
- Read season and timeline state
- Store namespaced rule data in the document

Rules cannot access files, network, environment variables, processes, native libraries or unrestricted clocks. The host supplies deterministic random streams when required.

### Rule manifest

```yaml
schema: canopy.rule/1
id: org.project-canopy.samples.crown-density
name: Crown Density
entry: crown_density.lua
controls:
  - id: density
    type: float
    minimum: 0.0
    maximum: 2.0
    default: 1.0
    label: Crown density
permissions:
  model.read: true
  model.write: true
limits:
  instructions: 200000
  memory_bytes: 8388608
```

### Lua API sketch

```lua
function on_control_changed(ctx, control_id)
    if control_id ~= "density" then return end
    local density = ctx.controls:get_number("density")
    for generator in ctx.model:find_generators({ tag = "crown" }) do
        generator:set("generation.frequency", density)
    end
end
```

All writes are atomic. An error rolls back the transaction and produces a source-located diagnostic.

## Python automation API

Python bindings expose the same public C++ domain objects through handles, value types and explicit transactions.

```python
from canopy import Document, EvaluateOptions, ExportPreset

with Document.open("Oak.canopyproj") as doc:
    with doc.transaction("Create production variants"):
        doc.set_seed(4217)
        doc.generator("crown.twigs").set("generation.frequency", 1.25)

    snapshot = doc.evaluate(EvaluateOptions(profile="preview"))
    snapshot.export(ExportPreset.load("presets/unreal.json"), "build/oak")
```

Binding rules:

- Python never owns raw engine pointers.
- Long operations release the global interpreter lock.
- Callbacks reacquire it on a documented thread.
- All errors become typed exceptions with structured diagnostics.
- Array data uses zero-copy buffers when safe.
- API documentation is generated from the same interface metadata as C++.

## Command-line automation

`canopy-cli` is the stable automation surface for CI and asset farms.

Commands include:

- `validate`
- `evaluate`
- `randomize`
- `render`
- `export`
- `compile-runtime`
- `package`
- `migrate`
- `inspect`
- `diff`
- `benchmark`
- `plugin list`
- `plugin verify`

Global behavior:

- Non-interactive by default
- Stable exit codes
- Human text, JSON or JSON Lines output
- Response files for long arguments
- Cancellation through signals
- Deterministic locale
- Atomic outputs
- No telemetry unless enabled

## Native plugin ABI

Plugins export a single query function:

```c
CANOPY_PLUGIN_EXPORT canopy_status canopy_plugin_query(
    const canopy_host_api* host,
    canopy_plugin_descriptor* descriptor);
```

The descriptor declares:

- ABI major and minor version
- Plugin identifier and semantic version
- Display name and vendor
- Capability list
- Required host features
- Thread-safety model
- Requested permissions
- Factory functions
- Shutdown function

Capabilities include:

- Generator type
- Geometry modifier
- Importer
- Exporter
- Validator
- Runtime packing recipe
- Viewport overlay
- Editor panel
- Asset provider
- Render backend

The ABI uses fixed-width types, explicit ownership, caller-provided allocators, opaque handles, UTF-8 spans and status objects. C++ wrappers are header-only conveniences and are not the compatibility boundary.

## Generator plugin contract

A generator plugin receives:

- Immutable parent node views
- Typed property values
- Named deterministic random streams
- Read-only asset handles
- A bounded geometry-builder API
- Cancellation and progress interfaces
- Evaluation profile and requested outputs

It returns generated nodes, geometry fragments, dependency records and diagnostics. It cannot mutate unrelated graph state. Generated object identifiers derive from the plugin ID, generator ID, parent ID and stable generation keys.

## Importer plugin contract

Importers receive a virtual input stream and import options. They return an intermediate scene containing meshes, curves, materials, images, skeletons, animation and metadata. Importers do not write directly into a document. The host validates and then applies the returned scene in one transaction.

## Exporter plugin contract

Exporters receive a read-only, normalized `ExportScene`, target profile, virtual output filesystem and cancellation token. They create files only beneath the assigned output root. Atomic commit is controlled by the host.

## Data-packing scripts

Vertex and mesh packing use sandboxed Lua over a restricted per-vertex API. Texture packing uses a declarative graph or a restricted shader-like expression language compiled by the host.

Packing recipes are versioned assets. They declare required input semantics, output declarations, defaults and validation vectors. Runtime assets embed the recipe identifier and final reflected layout.

## Plugin discovery

Discovery order:

1. Application-bundled signed plugins
2. System-wide administrator-installed plugins
3. User plugins
4. Project-local plugins when the project explicitly trusts them
5. CLI paths supplied for the current invocation

Identifiers are unique. The resolver never silently replaces a plugin with a different signer or incompatible major version.

## Trust and signing

Plugins may be unsigned in developer mode. Production mode can require signed manifests and binaries. The trust store records signer fingerprints and granted permissions.

Opening a project never auto-executes an unknown plugin. Missing plugins produce placeholders that preserve serialized data and explain the unavailable outputs.

## Out-of-process extension host

High-risk plugins run in `canopy-plugin-host`:

- Separate process and address space
- Versioned local RPC
- Shared-memory transfer for large immutable buffers
- Read-only input handles
- Output quotas
- CPU and wall-clock limits
- Cancellation and forced termination
- Crash diagnostics and automatic quarantine threshold

The main process remains able to save the document after an extension crash.

## Editor extensions

Editor plugins can add dock panels, commands, property editors, overlays and inspectors through declarative descriptors. They cannot replace core save, migration, undo or permission behavior.

UI extensions use host-provided widgets or a process-isolated web surface according to policy. Direct access to the Qt object tree is not a stable API.

## Hot reload

- Lua rules reload at transaction boundaries.
- Python modules reload only in developer sessions.
- Native plugins are not unloaded in-process after creating objects; developer hot reload uses a new extension-host process.
- Project serialization never depends on transient function pointers.

## API stability

- Rules API: semantic versioning with deprecation aliases for at least one major release.
- Python API: source compatibility within a major release.
- C ABI: backward compatible within a major ABI; old functions remain callable.
- C++ API: selected stable surface only; most implementation headers are private.
- File schemas: explicit migrations and conformance fixtures.

## Extension tests

The repository contains sample plugins for every capability. CI verifies:

- ABI negotiation
- Allocation and ownership rules
- Deterministic generator output
- Missing-plugin placeholders
- Permission denial
- Signature mismatch
- Extension-host crash recovery
- Output quota enforcement
- API examples against every supported language version

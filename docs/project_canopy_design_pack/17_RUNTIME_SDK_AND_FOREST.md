# Runtime SDK and forest system

## Purpose

The Canopy Runtime SDK loads compact vegetation assets, creates large instance populations, computes visibility and level of detail, prepares wind state, and emits renderer-neutral draw packets. It is a separate product boundary from the authoring core. Applications may use only the Core and Forest libraries, or add a supplied rendering backend.

The SDK must support individual hero plants and forests containing millions of instances. It must not require Qt, the Modeler, Python, Lua, or an authoring project.

## Library layering

| Library | Responsibility | Graphics API dependency |
|---|---|---|
| `canopy_runtime_core` | Memory mapping, schema validation, asset views, geometry streams, materials, animation metadata | None |
| `canopy_runtime_forest` | Populations, cells, culling, LOD, wind domains, shadow views, streaming | None |
| `canopy_runtime_render` | Renderer-neutral resource and draw interfaces | None |
| `canopy_runtime_webgpu` | Reference WebGPU backend | WebGPU |
| Engine adapters | Unity, Unreal, Godot, O3DE and custom engine bindings | Engine specific |
| `canopy_runtime_terrain` | Optional terrain tiles, splat masks, vegetation placement and streaming | None in core; backend optional |

Every lower layer is independently testable. `runtime_core` and `runtime_forest` are usable in server-side simulation builds.

## Runtime asset format

`.canopyrt` is a little-endian, independently documented, random-access binary package.

Required design properties:

- A fixed header with magic, schema version, feature flags, file size, section table offset, content hash and build identifier
- A section directory containing type, offset, compressed size, uncompressed size, alignment, compression, checksum and optional name
- 64-bit offsets and sizes
- Alignment suitable for direct GPU upload where possible
- Independent compression per section
- Memory-mappable uncompressed sections
- Unknown optional sections are skippable
- Unknown required sections reject the asset
- All indices, ranges, string offsets and decompression sizes are validated before exposure
- No executable code or unbounded recursive structure

Canonical sections:

| Section | Contents |
|---|---|
| `STR0` | UTF-8 string table |
| `META` | Species, author, source revision, license identifiers and custom metadata |
| `BND0` | Per-asset and per-LOD bounds, spheres and optional oriented boxes |
| `MAT0` | Material records and texture bindings |
| `TEX0` | Optional embedded texture payloads or external references |
| `GEO0` | Vertex and index stream descriptors |
| `VTXn` | Packed vertex buffers |
| `IDXn` | Index buffers and meshlet data |
| `LOD0` | LOD thresholds, draw ranges, fade data and geometry class masks |
| `IMP0` | Billboard/impostor atlas metadata and geometry |
| `WND0` | Wind profile, hierarchy and shader coefficient metadata |
| `GRW0` | Optional growth curves and topology-stable animation channels |
| `COL0` | Collision primitives and optional convex meshes |
| `ANC0` | Anchors, sockets, pivots and named attachment points |
| `SKL0` | Optional spine skeleton and skin data |
| `DBG0` | Optional diagnostics; stripped in shipping builds |

The normative schema belongs in `schemas/runtime/`. A Markdown description, JSON reflection description, binary conformance vectors and fuzz corpus ship with the SDK.

## Runtime asset views

Loading returns immutable views into mapped or owned memory. No hidden per-asset heap allocations occur after successful load unless a caller requests transcoding.

```cpp
namespace canopy::rt {

struct AssetLoadOptions {
    bool verify_section_checksums = true;
    bool permit_external_textures = true;
    bool retain_debug_sections = false;
    uint64_t maximum_uncompressed_bytes = 1ull << 34;
};

class AssetView {
public:
    [[nodiscard]] Version schema_version() const noexcept;
    [[nodiscard]] Bounds bounds() const noexcept;
    [[nodiscard]] std::span<const LodRecord> lods() const noexcept;
    [[nodiscard]] std::span<const MaterialRecord> materials() const noexcept;
    [[nodiscard]] std::span<const GeometryStream> geometry() const noexcept;
    [[nodiscard]] std::optional<WindProfileView> wind() const noexcept;
    [[nodiscard]] MetadataView metadata() const noexcept;
};

Expected<AssetView, LoadError> load_asset(
    std::span<const std::byte> bytes,
    const AssetLoadOptions& options);

}
```

A C ABI exposes opaque handles and plain-old-data structures. ABI-visible structures begin with `struct_size` and `api_version` fields so fields can be appended safely.

## Base assets and instances

A `SpeciesAsset` is immutable and shared. A `PlantInstance` stores only population-specific state:

- Stable 64-bit instance identifier
- Species handle
- Translation
- Orientation or an up/right frame
- Uniform or nonuniform scale according to profile
- Tint and seasonal offset
- Wind domain identifier
- User data index
- Visibility and shadow flags
- Optional growth age
- Optional previous transform for motion vectors

The common hot instance representation is structure-of-arrays. An application may supply its own instance storage by implementing `IPopulationSource`.

## Spatial population structures

The default forest uses a sparse hashed grid with configurable cell size. Large-world coordinates use a double-precision world origin plus float cell-local positions.

Each cell stores:

- Tight and conservative bounds
- Species-sorted instance spans
- Dirty generation counter
- Streaming residency state
- Optional hierarchical subcells for dense regions
- Current and previous visibility classifications

Alternative backends may implement loose octrees, BVHs, clipmaps or engine-native spatial partitions behind the same population interface. The hashed grid remains the normative reference implementation.

## Culling pipeline

The CPU reference pipeline is:

1. Convert camera and shadow views to a validated internal convention.
2. Determine candidate cells using a frustum versus cell-bounds test.
3. Optionally apply coarse horizon, portal or user-supplied occlusion.
4. Calculate conservative projected size for each visible instance.
5. Select continuous LOD and active geometry classes.
6. Classify instances into opaque, masked, transparent, billboard and shadow queues.
7. Produce stable, species-grouped draw packets.
8. Record visibility history for hysteresis and temporal effects.

A GPU-driven pipeline may upload candidate cells, perform instance and meshlet culling in compute, compact visible instances, and issue indirect draws. CPU and GPU modes must agree within documented tolerance on LOD transitions.

### Camera contract

The SDK never guesses conventions. `CameraView` explicitly declares:

- Handedness
- Clip-space depth range
- Reversed-Z use
- Infinite far plane use
- Projection type
- World-origin offset
- Row-major or column-major input

A validation helper renders or returns an asymmetric diagnostic frustum and basis to catch transposed matrices and axis mistakes.

## Level of detail

Each asset can contain continuous geometry LOD metadata, discrete baked meshes and an impostor terminal LOD. The runtime returns a normalized continuous value and one or two active discrete levels.

Selection supports:

- Projected screen size
- Distance scaled by field of view and viewport height
- Per-species and global bias
- Hysteresis
- Dithered or alpha crossfade
- Independent shadow LOD
- Optional geometry-class dropout
- Per-platform terminal LOD policy

LOD selection must be stable when the camera jitters. The previous frame classification is retained to prevent threshold chatter.

## Draw packet contract

The renderer-neutral output groups work without owning rendering resources:

```cpp
struct DrawPacket {
    SpeciesHandle species;
    MaterialHandle material;
    GeometryHandle geometry;
    uint32_t lod_index;
    GeometryClass geometry_class;
    BufferRange visible_instances;
    BufferRange indices;
    DrawFlags flags;
    SortKey sort_key;
};
```

The application maps asset stream semantics to its shader declarations. A reflection API exposes semantic name, storage type, normalization, component count, byte offset, stride and interpolation requirement. Load-time validation rejects a requested shader layout that cannot represent the asset.

## Resource interface

The reference render interface uses explicit capabilities rather than graphics-API assumptions:

- Create and destroy immutable or streaming buffers
- Create textures, samplers and views
- Create pipelines from supplied shader modules or caller-owned pipelines
- Record direct and indirect indexed draws
- Bind per-frame, per-view, per-species and per-draw resources
- Insert transitions and synchronization where required
- Query timestamp and pipeline statistics support

The preferred custom-engine integration consumes draw packets directly and bypasses the reference renderer.

## Wind domains

A forest may contain multiple wind domains, each with:

- Direction and velocity
- Gust strength, duration and spacing
- Turbulence and rolling-noise phase
- Height and spatial attenuation
- Transition time
- Optional weather-system identifier

The CPU wind controller updates a small state block per domain. Per-vertex movement remains shader based for real-time profiles. A no-wind state fully initializes every field to deterministic neutral values.

Wind state uses fixed-step integration internally. Applications pass absolute time or a monotonic delta. Large time jumps may reset phase according to policy to prevent numerical drift.

## Growth and seasonal runtime state

Runtime growth is optional. Assets can expose either:

- A scalar age that drives topology-stable vertex, scale and opacity channels
- Skeletal/spine animation data
- A baked frame sequence or point cache handled by the host

Seasonal variation selects or blends material sets, leaf occupancy, tints and dropped-foliage variants. Shipping assets must not change index topology during a continuous blend unless the target renderer explicitly supports topology swaps.

## Billboards and impostors

The runtime supports:

- Vertical camera-facing billboards
- Crossed cards
- Multi-view octahedral or hemispherical impostors
- Top-down cap views
- Per-view depth and normal reconstruction
- Dithered transition from geometry
- Correct world-space wind phase on impostors
- Per-instance hue and season variation
- Optional baked ambient occlusion and thickness

The impostor system reports camera basis and selected atlas frames but does not force a particular shader implementation.

## Shadows

Forest culling accepts an arbitrary set of shadow views. The reference cascaded-shadow helper calculates split frusta, stable light-space projections and per-cascade visibility. An engine may replace it entirely.

Shadow policies include:

- Independent LOD bias
- Geometry-class filtering
- Billboard shadow enablement
- Maximum shadow distance
- Per-species cast flags
- Alpha-tested shadow materials
- Two-sided or one-sided leaf shadowing

## Optional terrain module

The terrain module is not required for modeler parity but completes the runtime forest workflow. It provides:

- Tile and clipmap streaming
- Height and normal queries
- Layer/splat maps
- Density and exclusion masks
- Deterministic procedural instance placement
- Terrain-aligned instance orientation
- Population serialization
- User callbacks for physics and navmesh exclusions

The module does not dictate an engine terrain representation. It can ingest user-provided tile callbacks.

## Streaming and lifetime

Resource lifetime is explicit:

- File bytes may be caller owned, memory mapped by the SDK or copied by request.
- CPU asset views stay valid until asset release.
- GPU resources are owned by the backend or host.
- Unload is two-phase when frames may still reference resources.
- Streaming requests carry priorities and cancellation tokens.
- No callback is invoked while an internal lock is held.

## Threading

Read-only asset views are thread safe. Population mutations are recorded in command buffers and committed at a synchronization point. Culling jobs can run concurrently per view. The reference implementation uses a caller-provided job-system adapter and never creates unmanaged worker threads unless explicitly configured.

## Memory budgets

Applications can set budgets for:

- Mapped asset bytes
- Decompressed CPU geometry
- GPU geometry
- Textures
- Population cells
- Visible-instance buffers
- Transient culling storage

The SDK exposes current, peak and eviction-eligible bytes by category. Budget overruns generate structured events, not console-only text.

## Diagnostics

Per-frame statistics include:

- Resident and visible cells
- Candidate and visible instances
- LOD distribution by species
- Draw packets and material switches
- CPU culling time
- GPU culling and rendering timestamps when available
- Uploaded bytes
- Streaming misses
- Wind domains

A debug-draw interface emits frusta, cells, bounds, LOD labels, wind vectors and billboard selections.

## Runtime acceptance criteria

- A host can load a validated `.canopyrt` without the authoring libraries.
- One base asset can be instanced at least one million times without copying geometry per instance.
- CPU culling output is deterministic for identical inputs and worker counts.
- A GPU-driven sample renders the same test forest with equivalent visibility and LOD classifications.
- LOD hysteresis prevents oscillation under a subpixel camera perturbation test.
- Invalid sections, integer overflow, decompression bombs and out-of-range streams fail safely under fuzzing.
- A custom renderer can consume Core and Forest outputs without linking the reference rendering backend.
- Reference applications demonstrate one tree, a mixed forest, multi-domain wind, billboards, growth, metadata, collision and terrain streaming.

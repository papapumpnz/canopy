# ADR-0008: `.canopyrt` v1 — runtime format, compiler and CPU forest slice

- **Status**: Accepted
- **Date**: 2026-07-21
- **Design authority**: `06_DATA_MODEL_AND_FILE_FORMATS.md` (Runtime representation, Runtime file sections), `17_RUNTIME_SDK_AND_FOREST.md`

## Context

The runtime representation must be a compact, versioned binary generated from
an immutable production snapshot, readable without any authoring code, with
every section validated before use. The bootstrap slice needs discrete LOD
meshes, materials, bounds and metadata; meshlets, billboards, wind channels,
skeletons and collision sections arrive with later epics.

## Decision

1. **Container**: little-endian; header `CNPYRT\0\0`, u32 version (1), u32
   endianness probe (0x01020304), u64 feature flags (0), u32 section count;
   then a section table of `{u32 type, u32 subtype, u64 offset, u64 length,
   u64 sha256_low64}`. Every section is independently checksummed; readers
   validate magic, version, table bounds, checksums, and all cross-references
   before use, and reject unknown *required* feature flags.
2. **Sections (v1)**: METADATA(1) and MATERIALS(2) and LODS(3) are canonical
   JSON (self-describing, cheap to extend); VERTICES(4, subtype = LOD index)
   is interleaved float32 position/normal/uv (32 bytes per vertex);
   INDICES(5, subtype = LOD index) is uint32. The LODS descriptor lists, per
   LOD, primitives as `{material, index_offset, index_count}` into that
   LOD's buffers.
3. **Compiler**: authoring-side (`canopy_export`); bakes the draft, preview
   and production evaluation profiles as discrete LOD 2/1/0 of the same
   document at one timeline sample. Byte-deterministic like every exporter.
   Geometry merging is shared with the glTF writer (merge-by-material,
   semantic-node order).
4. **Runtime libraries**: `canopy_runtime_core` (loader) and
   `canopy_runtime_forest` depend on `canopy_foundation` only — never on
   document/evaluation/export. The bootstrap surface is C++; the stable C
   ABI wraps it in a later epic.
5. **Forest slice**: instances `{id, position, yaw, uniform scale}`;
   deterministic scatter from a named-stream seed with minimum spacing by
   rejection; LOD selection by distance bands derived from model bounds;
   distance visibility culling. Frustum culling, GPU paths and streaming are
   later work.

## Consequences

- A `.canopyrt` plus the runtime libs is enough to stand up a CPU forest
  with zero authoring code — the parity shape of the Runtime SDK.
- Discrete profile-baked LODs approximate continuous LOD until `14_LOD`
  lands; the format's LOD descriptor already carries per-LOD entries so the
  container will not change shape.
- JSON sections trade a few KB for extensibility; binary hot paths
  (vertices/indices) are raw typed buffers.

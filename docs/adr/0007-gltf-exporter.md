# ADR-0007: glTF 2.0 exporter (GLB), merge-by-material

- **Status**: Accepted
- **Date**: 2026-07-21
- **Design authority**: `16_EXPORT_PIPELINE.md` (industry-standard formats), `18_ENGINE_AND_DCC_INTEGRATIONS.md`, ADR-0001

## Context

OBJ was the bootstrap diagnostic format. Web and engine consumers want glTF;
`docs/integrations/three_js.md` promised it. glTF 2.0's binary container
(GLB) is a small, fully specified format writable without dependencies —
consistent with the zero-dependency policy. The full USD/Alembic/FBX adapter
work remains separate.

## Decision

1. Export preset `format: "gltf"` writes a single `.glb` (GLB v2: header +
   JSON chunk + BIN chunk, 4-byte aligned) plus the usual
   `.manifest.json`.
2. **Merge by material**: one glTF mesh with one primitive per referenced
   material (UUID order), vertices appended in semantic-node order.
   Per-node identity is not preserved in glTF output — it is a runtime
   interchange format; semantic-ID-addressable output remains the OBJ path
   until runtime metadata sections exist. Recorded in the manifest as
   `"granularity": "material"`.
3. Geometry: float32 POSITION/NORMAL/TEXCOORD_0 accessors with POSITION
   min/max, uint32 indices, Y-up right-handed meters (glTF's own
   convention — no axis conversion). Materials map to PBR
   metallic-roughness: `baseColorFactor` = season-blended material color,
   `metallicFactor` 0, `roughnessFactor` 0.9, `doubleSided` from the
   material, `alphaMode` BLEND when alpha < 1.
4. Deterministic bytes: same document + preset + timeline sample →
   byte-identical `.glb` (canonical JSON writer, fixed ordering, zero-filled
   padding).

## Consequences

- three.js and engine importers consume Canopy trees natively; the OBJ path
  stays for diagnostics and per-node addressing.
- float32 conversion is the first precision-lossy export; acceptable for
  runtime interchange and noted in the manifest (`"precision": "float32"`).
- Texture references arrive with the image pipeline; primitives are
  color-only until then.

# ADR-0004: Material model v1.1 and minor-version schema acceptance

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `06_DATA_MODEL_AND_FILE_FORMATS.md` (Schema evolution), `10_FOLIAGE_VINES_AND_DETAILS.md` (Leaf assets), `12_MATERIALS_TEXTURES_AND_PHOTOGRAMMETRY.md`

## Context

Bootstrap materials were id+name stubs. The leaf/frond slice needs real
material data: a display color, sidedness, and a leaf cutout outline (the 2D
polygon that gives a leaf its silhouette per `10_FOLIAGE` "Leaf assets").
Adding fields to `materials.json` is the project's first additive schema
change and must exercise the evolution policy from `06`.

## Decision

1. `materials.json` entries gain optional fields (defaults preserve v1.0
   semantics):
   - `base_color`: `[r, g, b, a]`, linear floats in [0, 1]; default
     `[0.5, 0.5, 0.5, 1]`.
   - `two_sided`: boolean, default `false`.
   - `cutout`: `{ "stem": [x, y], "vertices": [[x, y], ...] }` — a simple
     polygon in normalized leaf space (y 0→1 stem→tip, x −0.5→0.5 across),
     ≥ 3 finite vertices, triangulated at evaluation by ear clipping.
2. Authoring `schema_version` becomes `1.1.0`. Readers accept any `1.x`
   document: minor versions are additive-with-defaults, and unknown fields
   are ignored per the canonical-JSON policy, so forward minor reads are
   also safe. Major ≠ 1 is rejected with `unsupported_version`.
3. Exported `.mtl` files carry `Kd` from `base_color` — the first end-to-end
   material path from document to consumer.

## Consequences

- Checked-in 1.0.0 fixtures (`MinimalTrunk`) keep loading unchanged; a
  compatibility test pins this.
- Ear-clipping handles simple polygons only; self-intersecting outlines are
  rejected with a diagnostic. Texture-based cutout extraction arrives with
  the image pipeline (`12_MATERIALS...`).

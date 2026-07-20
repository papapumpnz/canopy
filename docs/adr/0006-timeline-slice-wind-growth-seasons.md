# ADR-0006: Timeline slice — wind, growth and seasons before manual edits

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `13_WIND_GROWTH_AND_SEASONS.md`, `34_BOOTSTRAP_BACKLOG.md` sequence

## Context

Sequence item 5 (manual edit layers *and tool interaction*) presumes the
interactive tooling deferred by ADR-0005. Item 6 (wind, growth, seasons) is
fully headless, deterministic, and exportable through the existing diagnostic
path — `13` explicitly lists "baked per-frame meshes for diagnostic use" as a
wind export mode.

## Decision

1. Take the first slice of item 6 now; manual edit layers follow when the
   command/tooling layer exists.
2. Evaluation accepts a `TimelineSample { time_s, growth, season, wind }`.
   Same document + profile + sample → identical output (frame-exact
   determinism per `13` "Deterministic frame sampling").
3. **Wind** (authoring preview, VFX-style): per-node deterministic
   oscillators — phase from a named stream, amplitude from hierarchy depth
   and generator kind (fronds/leaves livelier than boughs), gust envelope —
   applied as hierarchical rigid rotations about each node's base, parent
   transforms composed onto descendants. The trunk base never moves. Leaf
   batches additionally get a small deterministic vertex ripple. Semantic
   wind *vertex channels* for game export arrive with the runtime compiler.
4. **Growth**: per-generator `growth.start`/`growth.duration` with defaults
   derived from generator depth (parent-relative timing). A node's length
   and radius scale by its smoothstepped growth factor; children attach to
   the *grown* parent spine, so attachments move with growth as `13`
   requires. Nodes below the visibility threshold are omitted.
5. **Seasons**: material `season_color` (additive schema → authoring version
   1.2.0) blends leaf colors in the exported MTL; per-leaf drop uses a named
   stream so mottling is deterministic and monotonic in season. Dropped
   leaves are omitted (ground-debris batches are later work).

## Consequences

- CLI gains `--time/--growth/--season/--wind-strength/--wind-direction`;
  frame sequences are shell loops over `--time` until the timeline object
  lands.
- Rigid per-node sway approximates bending (no intra-branch flex yet); good
  for preview and baked diagnostics, revisited with runtime wind channels.
- Season color is per-material in MTL (global blend); per-node color
  mottling needs per-node material instancing at export — recorded as a gap.

## Amendment (2026-07-21)

Rigid per-node sway made trunks and ground-clamped roots visibly rock at
ground level (defect reported by the supervisor). Wind now applies a
**quadratic stiffness falloff** per vertex — zero displacement at each node's
base, full tip angle at its end — composed hierarchically by evaluating the
parent's bend at the child's attachment distance. Ground-clamped nodes have
zero amplitude, and their near-base attachment inherits ~zero motion through
the same falloff. Foliage vertices ride their parent branch's bend directly.
The exported `_WIND_PARAMS.z` now carries the bend length (hierarchy depth is
baked into the amplitude), and `canopy-wind.js` mirrors the falloff, keeping
shader wind equal to baked wind.

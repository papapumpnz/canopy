# ADR-0005: Defer the viewport milestone; proceed with generation modes

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `34_BOOTSTRAP_BACKLOG.md` ("Next sequence after bootstrap"), ADR-0001

## Context

The post-bootstrap sequence lists "WebGPU viewport and Qt command/view-model
shell" as item 3. That milestone requires two large dependency intakes — Qt 6
and `wgpu-native` (which itself needs a Rust toolchain) — neither of which is
available in the current development environment, and both of which require
the full intake record process from `05_REPOSITORY_AND_BUILD.md` plus network
access for reproducible source fetches. The backlog permits reordering when an
ADR records the dependency rationale.

## Decision

1. Defer the viewport/Qt milestone until dependency intake for Qt 6 and
   `wgpu-native` can be performed properly (locked sources, license records,
   CI provisioning). The headless-first architecture (ADR-0001) means no core
   work is blocked by this.
2. Proceed with sequence item 4, "full generation modes and remaining core
   generators", starting with the placement modes from
   `07_EVALUATION_ENGINE.md`: phyllotaxy, proportional, and bifurcation.
3. Diagnostic rendering continues via the OBJ export path until the viewport
   lands.

## Consequences

- Interactive authoring remains CLI/document-driven for now; the Qt shell
  arrives later with its own work units.
- Generation-mode work deepens the deterministic core that the viewport will
  consume unchanged, so no rework is expected from the reordering.

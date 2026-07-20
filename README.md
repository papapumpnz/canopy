# Project Canopy

Clean-room vegetation authoring, animation, export, and runtime platform.

This repository implements the design pack in `docs/project_canopy_design_pack/`.
Read `docs/project_canopy_design_pack/00_README.md` first, then
`34_BOOTSTRAP_BACKLOG.md` for the current implementation sequence.

## Status

Bootstrap phase (Stages 0–4 of `34_BOOTSTRAP_BACKLOG.md`): deterministic vertical
slice from a `.canopyproj` document through evaluation and branch meshing to an
OBJ export driven by `canopy-cli`.

## Building

```text
cmake --preset headless-dev
cmake --build --preset headless-dev
ctest --preset headless-dev
```

Requires a C++20 compiler, CMake ≥ 3.25, and Ninja. The headless presets have no
GUI dependencies; Qt is only required for the (future) modeler application.

## Layout

```text
apps/            canopy-cli, modeler, runtime-viewer (applications only)
src/foundation/  diagnostics, IDs, hashing, canonical JSON, random streams, curves
src/document/    document model, schemas, canonical serialization, project I/O
src/evaluation/  deterministic evaluation kernel and generators
src/geometry/    splines, frames, spine sampling, branch meshing
src/export/      export scene and format writers
src/runtime/     runtime SDK (not yet started)
schemas/         JSON schemas for persisted formats
tests/           unit and integration tests plus fixtures
docs/            design pack and ADRs
agent-runs/      machine-readable provenance records for agent contributions
```

## Governance

See `GOVERNANCE.md`. All contributions—human or agent-assisted—carry provenance
records; public schema or API changes require an ADR in `docs/adr/`.

"Project Canopy" is a codename only.

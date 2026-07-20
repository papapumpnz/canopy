# Agent guide

Read `docs/project_canopy_design_pack/24_AGENT_IMPLEMENTATION_PLAYBOOK.md` before
making changes. This file is the operational digest.

## Build and test

```text
cmake --preset headless-dev          # configure (no GUI deps)
cmake --build --preset headless-dev  # build
ctest --preset headless-dev          # run all tests
cmake --preset headless-san          # ASan+UBSan configure
```

Binaries land in `build/<preset>/`. The CLI is `build/<preset>/apps/cli/canopy-cli`.

## Hard rules

- C++20, warnings are errors. No exceptions across C/plugin boundaries.
- No raw owning pointers; RAII everywhere.
- Determinism is observable contract: named random streams only (never a global
  RNG), stable iteration order wherever output is observable, canonical JSON for
  everything persisted (sorted keys, fixed numeric formatting).
- No hidden I/O in evaluation code. No new dependencies without an ADR.
- Schema or public-API changes require an ADR in `docs/adr/` in the same change.
- Tests first: a change is not done because it compiles. Bug fixes need a
  regression test that failed before the fix.

## Layout contract

Dependency direction (lower may not include higher):

```text
foundation → document → geometry → evaluation → export → apps
```

`src/runtime/` must not depend on authoring libraries. `apps/` may depend on
anything; nothing depends on `apps/`.

## Provenance

Agent-assisted changes add a run record under `agent-runs/` (see
`agent-runs/README.md`) and reference it from the PR description.

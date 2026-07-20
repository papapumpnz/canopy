# ADR-0001: Bootstrap repository layout and zero-dependency policy

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `34_BOOTSTRAP_BACKLOG.md` B-004, `05_REPOSITORY_AND_BUILD.md`

## Context

The pack contains two layouts: `05_REPOSITORY_AND_BUILD.md` (`src/base`,
`src/eval`, apps under `src/`) and `34_BOOTSTRAP_BACKLOG.md` B-004
(`apps/`, `src/foundation`, `src/evaluation`). They conflict. The bootstrap
backlog is the explicit first implementation sequence with acceptance criteria,
and its layout separates applications from libraries more cleanly.

The dependency policy in `05` requires intake records for every dependency.
The bootstrap vertical slice (document → evaluation → meshing → OBJ) needs
JSON, hashing, PRNG, and math — all small, stable, and implementable in-house.

## Decision

1. Adopt the B-004 layout (`apps/`, `src/foundation|document|evaluation|
   geometry|materials|export|runtime`). `05_REPOSITORY_AND_BUILD.md` target
   *names* (`canopy_foundation`, `canopy-cli`, …) are kept.
2. The bootstrap has **zero third-party runtime dependencies**. Canonical JSON,
   SHA-256, UUID, PRNG, and curve math are implemented in `src/foundation` with
   exhaustive tests. Test harness is a minimal in-house `canopy_test` header
   registered with CTest (Catch2/GoogleTest intake deferred to a later ADR).
3. Dependency direction: `foundation → document → evaluation → geometry →
   export → apps`; runtime libraries never depend on authoring libraries.

## Consequences

- No network access or vendoring needed to build; reproducibility is trivial.
- In-house JSON/SHA-256 must be fuzz- and property-tested (tracked in backlog
  B-008/B-010); they are small, bounded, and replaceable behind interfaces.
- When Catch2 or RapidCheck-style testing is adopted, tests migrate but the
  CTest entry points remain stable.

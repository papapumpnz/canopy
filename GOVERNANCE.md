# Governance

Authority for scope and architecture is the design pack in
`docs/project_canopy_design_pack/`. Where this file and the pack disagree, the
pack wins and this file must be corrected.

## Clean-room policy

This is a clean-room implementation. Contributors and agents must not:

- Copy, decompile, or paraphrase proprietary SpeedTree source code, shaders,
  schemas, file-format internals, or UI artwork.
- Introduce assets or documents derived from proprietary sample content.
- Implement readers or writers for proprietary formats (`.spm`, `.st`, `.st9`,
  `.stsdk`, `.ste`). Such support may only arrive as a separately licensed,
  vendor-approved adapter (see `28_CLEAN_ROOM_AND_LICENSING.md`).

Publicly documented behavior (manuals, marketing pages, published papers) may be
used to define parity targets. Record consulted sources in the provenance
record of the contribution.

## Contribution provenance

Every pull request must state:

- **Author type**: human, agent-assisted, or agent-authored under review.
- **Agent run record**: for agent contributions, a record in `agent-runs/`
  conforming to `agent-runs/run-record.schema.json`.
- **Restricted-material declaration**: confirmation that no proprietary or
  license-incompatible material informed the change.
- **Design authority**: the design-pack sections and ADRs the change implements.

CI rejects changes to `schemas/` or public headers that do not reference an ADR.

## Decision records

Architectural decisions live in `docs/adr/`. A decision is required for: public
API or schema changes, new dependencies, determinism-affecting algorithm
changes, and golden-baseline updates. See `docs/adr/README.md`.

## Dependencies

Every third-party dependency requires an intake record (exact version lock,
source URL, integrity hash, license text, maintenance status, ABI exposure) per
`05_REPOSITORY_AND_BUILD.md`. The bootstrap deliberately has **zero** runtime
third-party dependencies; adding one requires an ADR.

## Review

- Main is always buildable; warnings are errors in CI.
- Golden or fixture updates require the diff report described in
  `24_AGENT_IMPLEMENTATION_PLAYBOOK.md` and separate reviewer approval.
- Determinism is a contract: any change to observable output (hashes,
  topology, serialized bytes) must be called out explicitly in the PR.

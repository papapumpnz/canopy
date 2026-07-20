# ADR-0003: Named random streams

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `07_EVALUATION_ENGINE.md` (Stable random service), `34_BOOTSTRAP_BACKLOG.md` B-018

## Context

Every observable random decision must be reproducible, unaffected by unrelated
edits, worker count, or the addition of later streams.

## Decision

- Stream key = SHA-256 over the domain-separated tuple
  `("canopy-rng-v1", document_seed, generator_id, semantic_parent_id,
  property_key, purpose_tag, algorithm_version)`; the first 8 bytes (little
  endian) seed the stream.
- Generator: SplitMix64 (public-domain algorithm, constants from Steele et al.)
  — counter-based, splittable by construction of new named keys, cross-platform
  exact.
- Each *decision* consumes explicitly numbered draws from its own stream; code
  never shares a stream across unrelated decisions.
- `algorithm_version` starts at 1 and increments only with an ADR, because it
  changes every downstream value.
- Distribution mapping: `next_double()` = 53-bit mantissa / 2^53 in [0,1);
  uniform ranges map by scale-offset; other distributions are defined in
  `foundation/random.hpp` with fixed algorithms (no `std::` distributions,
  which are implementation-defined).

## Consequences

- Adding a new stream or reordering evaluation cannot shift existing values
  (B-018 acceptance).
- SplitMix64 is not cryptographic — acceptable: streams drive art-directable
  variation, not security. Key derivation uses SHA-256 only for stable mixing.

# ADR-0002: Canonical JSON and content-hash rules

- **Status**: Accepted
- **Date**: 2026-07-20
- **Design authority**: `06_DATA_MODEL_AND_FILE_FORMATS.md` (Canonical JSON), `34_BOOTSTRAP_BACKLOG.md` B-013

## Context

Repeated save must be byte-identical across platforms, worker counts, and
working directories, and the document content hash must be stable.

## Decision

Canonical JSON emitted by `canopy::json::write_canonical`:

- UTF-8, no BOM. Object keys sorted by byte-wise comparison of UTF-8 keys.
- Arrays keep semantic order (caller responsibility).
- Two-space indentation, `\n` line endings, no trailing whitespace, single
  trailing newline at end of file.
- Strings escape `"` `\\` and control characters (`\n` `\t` `\r` `\b` `\f`,
  otherwise `\u00XX`); all other code points emitted verbatim as UTF-8.
- Numbers: integers in `[-2^53, 2^53]` print as integers. Other finite doubles
  print via shortest round-trip (`std::to_chars`, `double` precision). NaN and
  infinity are serialization errors, never silently written.
- No comments; unknown fields preserved verbatim under their namespaced keys.

Content hash: SHA-256 over the canonical bytes of each document file,
domain-separated as `sha256("canopy-file-v1\0" + path_in_project + "\0" + bytes)`,
and the document hash is SHA-256 over the sorted list of `path\0filehash`
entries prefixed `"canopy-doc-v1\0"`. Hashes render as lowercase hex.

## Consequences

- Semantic no-op edits (key reordering) do not change bytes or hashes.
- Shortest-round-trip float printing is locale-independent and stable across
  conforming `to_chars` implementations; CI must include a cross-platform
  vector test (B-009 acceptance).

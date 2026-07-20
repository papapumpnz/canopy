# Agent run records

Machine-readable provenance for agent-assisted contributions (backlog B-001).
One JSON file per run: `YYYYMMDD-<short-topic>-<n>.json`, validating against
`run-record.schema.json`. PRs reference the record path in their provenance
section. CI fails agent-attributed changes without a record.

A record states what the agent was asked to do, which design-pack sections it
consulted, what it produced, how it was validated, and a restricted-material
declaration.

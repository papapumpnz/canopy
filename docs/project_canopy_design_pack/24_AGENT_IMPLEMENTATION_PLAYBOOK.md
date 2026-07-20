# AI-agent implementation playbook

## Purpose

This playbook converts the design pack into small, reviewable implementation work for coding agents. Agents are treated as fast contributors, not autonomous product owners. Architectural decisions, security boundaries, public API changes, golden updates and release claims require designated review.

## Work-unit contract

Every agent task must contain:

- One issue identifier
- Objective and user-visible outcome
- In-scope files or modules
- Explicit non-goals
- Applicable design sections and ADRs
- Inputs and expected outputs
- Acceptance tests
- Commands to build and test
- Performance or determinism constraints
- Dependency and licensing constraints
- Definition of done

Tasks should normally fit one coherent change. Splitting by file count alone is discouraged; split by independently testable behavior.

## Agent roles

### Architecture agent

Produces interfaces, dependency direction, ADR proposals and migration plans. It does not merge speculative abstractions without a concrete first consumer.

### Domain algorithm agent

Implements generators, curves, meshing, collision, wind, growth, LOD or packing. It must provide invariants, numerical edge cases, tests and a benchmark.

### Platform agent

Owns build systems, OS integration, graphics backend, packaging or host integration. It must state the tested platform and avoid changing cross-platform contracts casually.

### UI agent

Implements Modeler presentation over existing commands and view models. It may not bypass transactions, validation or undo.

### Test agent

Builds adversarial, property-based, golden, fuzz and performance coverage. It does not alter production behavior merely to make an incorrect expectation pass.

### Reviewer agent

Checks a patch against design, public API, security, determinism and tests. It reports concrete findings by severity and does not rewrite unrelated code.

### Documentation agent

Updates public and internal documentation from implemented behavior. Examples must be executable where possible.

## Context packet template

````markdown
# Task CAN-XXXX: <title>

## Objective
<one paragraph>

## Design authority
- `08_GENERATORS.md`, section ...
- `29_ARCHITECTURE_DECISIONS.md`, ADR-...

## In scope
- `src/core/...`
- `tests/...`

## Non-goals
- ...

## Existing contracts
- Interface: ...
- Threading: ...
- Ownership: ...
- Determinism: ...

## Acceptance
- [ ] ...

## Commands
```text
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R <pattern>
```

## Required handoff
- Summary
- Files changed
- Tests and results
- Open risks
````

## Task execution sequence

1. Read the task, cited design sections, neighboring interfaces and tests.
2. Restate the observable contract in a short implementation note inside the issue or patch description.
3. Identify edge cases and failure behavior before coding.
4. Add or update a failing test that demonstrates the required behavior.
5. Implement the smallest coherent production change.
6. Run focused tests, then module tests, then required repository checks.
7. Inspect generated diffs and artifacts; do not assume generation succeeded.
8. Benchmark when the task touches a hot path.
9. Update documentation and schema/API baselines in the same change when behavior changes.
10. Produce a structured handoff.

## Repository navigation rules

- Search for the public contract before inventing a new type.
- Prefer module-local interfaces over cross-layer include dependencies.
- Do not expose private implementation headers to solve a build error.
- Do not duplicate math or serialization utilities.
- Follow existing naming, error and ownership patterns.
- Generated files must identify their generator and must not be manually edited.
- New dependencies require an ADR or dependency review record.

## Coding rules

- C++20, warnings treated as errors in CI.
- RAII for ownership.
- No raw owning pointers.
- No exceptions across C or plugin boundaries.
- Explicit units and coordinate conventions.
- Checked conversion between integer widths.
- Stable iteration order where output is observable.
- No global mutable random generator.
- No hidden I/O in evaluation code.
- No blocking work on the GUI thread.
- Cancellation and diagnostics in expensive operations.
- Public functions document thread safety and lifetime.

## Numerical implementation checklist

For geometry or simulation work, the agent must state:

- Input domain and units
- Degenerate input policy
- Chosen epsilon or robust predicate and its scale rationale
- Frame/axis convention
- Determinism implications
- Complexity
- Memory behavior
- Cancellation points
- Invariants verified by tests

Avoid broad clamping that hides bad data. Emit diagnostics when repair changes meaning.

## Test-first expectations

A task is not done because it compiles. Required tests depend on change type:

| Change | Minimum evidence |
|---|---|
| Pure function | Unit and edge-case tests |
| Geometry algorithm | Unit, property/invariant and golden asset test |
| Parser/schema | Valid, invalid, boundary and fuzz seed tests |
| UI command | Command-model test and focused UI wiring test |
| Runtime hot path | Correctness comparison and benchmark |
| Public API | Compile examples, docs and compatibility baseline |
| Integration | Automated host smoke test or a documented controlled test fixture |
| Bug fix | Regression test that failed before the change |

## Golden-update rule

An agent may generate candidate golden changes but may not label them correct merely because all new outputs agree with the patch. The handoff must include:

- Exact changed metrics or images
- Intended behavior change
- Why old output was wrong or obsolete
- Any downstream compatibility effect

A reviewer approves the baseline update separately.

## Dependency changes

An agent proposing a dependency must report:

- Capability needed
- Considered alternatives
- License and linking implications
- Supported platforms
- Security and maintenance status
- Binary and source size impact
- Reproducible-build method
- Isolation plan

A convenience library is not accepted solely to avoid implementing a small stable utility.

## Public contract changes

Before changing API or schema:

1. Search current consumers.
2. State compatibility impact.
3. Add or update an ADR.
4. Provide migration or deprecation path.
5. Update examples, bindings and reflection.
6. Run ABI/schema checks.

Agents must not renumber serialized enums or repurpose fields.

## Pull request structure

```markdown
## Outcome
<observable result>

## Design
<important choices and alternatives>

## Validation
- Tests: ...
- Benchmark: ...
- Platforms: ...

## Compatibility
- API: none / details
- Schema: none / details
- Golden output: none / details

## Risks and follow-up
<bounded remaining items>
```

Keep refactors separate from behavior changes unless the refactor is necessary and directly reviewed.

## Review checklist

A reviewer checks:

- Matches issue and design authority
- Correct dependency direction
- Deterministic identities and ordering
- Ownership, lifetime and threading
- Error and cancellation behavior
- Numerical edge cases
- Security of untrusted input
- Test quality and negative coverage
- Performance in hot paths
- Public API/schema compatibility
- Documentation accuracy
- Third-party provenance

Findings are classified as blocker, major, minor or observation. Every blocker and major finding includes a concrete failure scenario.

## Handoff template

```markdown
# Handoff CAN-XXXX

## Implemented
- ...

## Changed files
- `path`: purpose

## Validation
- Command: `...`
- Result: ...

## Determinism/performance
- ...

## Decisions
- ...

## Unresolved risks
- ...
```

## Parallel work

Parallel agents may work only when ownership boundaries are clear. Use interface stubs and contract tests to decouple work.

Safe parallel examples:

- Generator implementations behind a stable generator interface
- Independent format exporters consuming `ExportScene`
- Engine integrations consuming a versioned manifest
- UI panels over stable commands
- Tests and documentation for an already fixed contract

Unsafe parallel examples:

- Multiple agents altering the same core schema
- Geometry and LOD agents independently redefining vertex identity
- Runtime and exporter agents changing packing semantics without a shared contract
- UI work against an interface still being redesigned

## Merge train

Changes enter in dependency order:

1. ADR or contract
2. Interface and conformance test
3. Core implementation
4. Bindings and UI
5. Integration and documentation
6. Performance tuning

Feature branches rebase onto the latest contract before final review. A merge queue runs full determinism, schema and ABI checks.

## Agent prompt: implementation

```text
You are implementing task <ID> in Project Canopy.
Read the supplied task and its cited Markdown design sections before editing.
Preserve public APIs and serialized schemas unless the task explicitly authorizes a reviewed change.
Implement the smallest complete behavior, with regression and edge-case tests.
Use deterministic ordering and named random streams for observable output.
Do not add dependencies, hidden I/O, GUI-thread blocking, or unbounded allocations.
Run the listed commands and report exact results.
Return a handoff using the repository template, including risks and compatibility effects.
```

## Agent prompt: review

```text
Review patch <ID> against its task, cited design, public contracts and tests.
Prioritize correctness, data loss, nondeterminism, security, lifetime/threading, numerical robustness, API/schema compatibility and hot-path regressions.
Do not focus on cosmetic style unless it causes maintenance or correctness risk.
Report findings by severity with file/line, failure scenario and required correction.
State which tests or assumptions you independently verified.
```

## Completion rule

An epic is complete only when its acceptance tests, documentation, diagnostics and required performance evidence exist. A collection of compiled stubs is not feature parity.

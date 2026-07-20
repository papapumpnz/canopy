# Risk register

## Scoring

Likelihood and impact use 1–5. Exposure is their product. Scores are reviewed at each release gate. A mitigation is not complete until evidence exists.

| ID | Risk | L | I | Exposure | Mitigation | Trigger / evidence |
|---|---|---:|---:|---:|---|---|
| R-001 | “Same functionality” expands without a controlled parity baseline | 4 | 5 | 20 | Freeze public baseline per release; trace every capability and change | New official feature or unowned parity row |
| R-002 | Attempted proprietary file compatibility contaminates clean-room work | 3 | 5 | 15 | Exclude native formats; separate licensed adapter process if ever authorized | Patch references reverse-engineered fields or proprietary samples |
| R-003 | Generator scope overwhelms architecture before the core stabilizes | 4 | 5 | 20 | Contract-first generator API; vertical slices; shared tests; epic gates | Multiple generators add incompatible property/geometry models |
| R-004 | Determinism breaks under parallel evaluation | 4 | 5 | 20 | Named streams, stable IDs/order, deterministic reductions, worker-count matrix | Topology hash differs between worker counts |
| R-005 | Floating-point behavior diverges across CPU architectures | 4 | 4 | 16 | Define exact versus tolerant fields; avoid unstable predicates; cross-arch CI | ARM64 and x86-64 exceed field tolerance |
| R-006 | Branch junction meshing creates cracks, self-intersections or unstable normals | 4 | 4 | 16 | Dedicated robust meshing layer, pathological corpus, invariants and visual tests | Degenerate junction rate or silhouette regression rises |
| R-007 | Vine physics is slow or nondeterministic | 3 | 4 | 12 | Authoring-only fixed-step constrained solve; bounded iterations; bake result | Variant differs by thread count or exceeds solve budget |
| R-008 | Scan conversion quality cannot handle real-world noisy data | 4 | 4 | 16 | Modular cleanup/extraction, quality metrics, multiple strategies, manual override | Synthetic and licensed scan corpus fails axis/feature metrics |
| R-009 | Texture and impostor baking differs across GPUs | 4 | 3 | 12 | CPU reference, deterministic offline profile, conformance captures | Bake hashes or pixels exceed target across backend |
| R-010 | LOD changes identity and breaks edit, wind or seasonal mappings | 4 | 5 | 20 | Stable source IDs, explicit remap tables and LOD contract before simplification | An element has no deterministic survivor/remap record |
| R-011 | Real-time wind semantics diverge between viewport and engines | 4 | 4 | 16 | Shared semantic spec and vectors; engine shader conformance images | Fixed-time deformation mismatch exceeds threshold |
| R-012 | VFX growth with topology changes is unreliable across formats | 3 | 4 | 12 | Separate topology-stable and varying paths; USD/Alembic reference validation | Empty frame, timing or topology sequence import fails |
| R-013 | Qt or WebGPU platform behavior blocks a host | 3 | 4 | 12 | Thin platform adapters; native fallbacks for critical surfaces; early CI | Viewport or docking fails on a supported authoring platform |
| R-014 | Shader compiler or driver variation causes runtime defects | 4 | 4 | 16 | Precompiled/tested variants, reflection validation, backend matrix | Driver crash, layout mismatch or unexplained visual drift |
| R-015 | Runtime CPU culling cannot meet large-forest budget | 3 | 5 | 15 | Cell structure benchmarks, SIMD where justified, GPU path, user job system | Candidate count or frame time exceeds profile |
| R-016 | GPU-driven rendering couples the SDK to one renderer | 3 | 5 | 15 | Renderer-neutral draw packets and reference backend separation | Core/Forest starts depending on WebGPU objects |
| R-017 | `.canopyrt` schema becomes brittle or unsafe | 3 | 5 | 15 | Sectioned format, reflection, size limits, fuzzing, optional/required flags | Reader requires unchecked casting or unbounded allocation |
| R-018 | C ABI breaks downstream engines | 3 | 5 | 15 | Opaque handles, struct sizes, ABI dumps, old-client test binaries | Symbol/layout comparison changes within major ABI |
| R-019 | Optional FBX support creates licensing or CI constraints | 4 | 3 | 12 | Isolated adapter, open formats canonical, licensed builders only | Core build or tests begin requiring FBX SDK |
| R-020 | DCC/engine release churn causes integration maintenance overload | 5 | 4 | 20 | Shared manifests, generated mappings, explicit support matrix, maintenance branches | Host API change breaks more than one integration layer |
| R-021 | Asset library becomes a network dependency | 3 | 4 | 12 | Offline-first local catalog and lockfile; remote service optional | Model open/evaluation blocks on registry availability |
| R-022 | Third-party or sample content lacks distribution rights | 3 | 5 | 15 | Provenance required, original corpus, automated manifest checks, audit gate | Missing license/source fields or unverifiable scan provenance |
| R-023 | Rules or plugins execute untrusted code on project open | 4 | 5 | 20 | Sandboxed Rules, explicit trust, signatures and process isolation | Unknown plugin or script runs without consent |
| R-024 | Extension process can exhaust memory/CPU or escape output root | 3 | 5 | 15 | Quotas, cancellation, shared-memory rules, virtual output FS, sandbox | Extension exceeds declared quota or creates outside path |
| R-025 | Incremental cache serves stale geometry | 3 | 5 | 15 | Complete dependency fingerprints, trace assertions and cold/warm equivalence | Warm result differs from forced cold evaluation |
| R-026 | Save/migration failure loses source data | 2 | 5 | 10 | Copy-on-write migration, atomic replace, journals and fault injection | Source file changes after simulated failed save |
| R-027 | UI implements hidden behavior outside headless core | 4 | 4 | 16 | Command/view-model boundary and headless command tests | Workflow works only by direct widget-side mutation |
| R-028 | AI agents make broad inconsistent changes | 5 | 4 | 20 | Small work units, design authority, module ownership, merge queue and reviewer agents | Patch changes unrelated modules or public contracts without ADR |
| R-029 | Generated code or docs drift from interfaces | 3 | 3 | 9 | Single-source generation and CI regeneration diff | Generated files differ after clean regeneration |
| R-030 | Performance targets are claimed without reproducible conditions | 4 | 4 | 16 | Hardware/profile manifests and signed benchmark output | Result omits hardware, driver, assets or feature settings |
| R-031 | Accessibility is deferred until architecture is rigid | 4 | 3 | 12 | Accessible view models and keyboard paths from first Modeler gate | Core panels lack names, focus order or keyboard commands |
| R-032 | Cross-platform packaging or code signing blocks releases | 3 | 4 | 12 | Early notarization/signing prototypes and reproducible package pipeline | RC artifact cannot install on clean supported system |
| R-033 | Dependency abandonment or vulnerability disrupts product | 3 | 4 | 12 | Minimize dependencies, pin, monitor, isolate and maintain replacement plan | Unsupported version or critical advisory with no patch path |
| R-034 | Marketing overstates parity or compatibility | 3 | 5 | 15 | Evidence-derived claims, approved terminology and legal review | Claim uses “drop-in,” native format support or unsupported host version |
| R-035 | Original implementation accidentally copies public UI expression too closely | 2 | 4 | 8 | Workflow parity, independent interaction design and visual review | Side-by-side design review finds distinctive copied arrangement/artwork |

## Review procedure

At each gate:

1. Re-score likelihood and impact.
2. Attach test, benchmark, audit or design evidence to active mitigations.
3. Convert triggered risks into tracked defects or ADRs.
4. Add newly discovered risks; do not hide them inside task notes.
5. Identify risks accepted for the release and state the user-visible limitation.

## Escalation thresholds

- Exposure 20–25: executive/architecture owner and release owner must approve mitigation plan.
- Exposure 15–19: epic owner must provide evidence before the next gate.
- Exposure 8–14: tracked and reviewed at normal milestone cadence.
- Exposure below 8: monitored; may still be release blocking when security, data integrity or licensing is involved.

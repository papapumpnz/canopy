# Procedural evaluation engine

## Objectives

The evaluator transforms a document revision into immutable geometry, material, animation, optimization, and diagnostic snapshots. It must support interactive partial recomputation and deterministic production builds.

## Evaluation phases

1. **Validation**: graph structure, assets, property ranges, references, plugin availability
2. **Property resolution**: rules, expressions, curves, variance, overrides, timeline sample
3. **Candidate generation**: attachment positions and orientations
4. **Spine or placement solve**: branch paths, vine constraints, mesh attachments
5. **Geometry construction**: branch sweeps, leaves, details, imported meshes
6. **Force and collision passes**: attract, avoid, obstruct, crawl, align, leaf collision
7. **Manual edit replay**: spine, vertex, trim, displacement, paint layers
8. **Post processes**: shade pruning, AO, normals/tangents, UV checks
9. **LOD derivation**: continuous annotations and preview state
10. **Snapshot publication**: immutable buffers, semantic maps, diagnostics, statistics

A generator declares which phases it participates in and which outputs it reads.

## Graph scheduling

The generator graph is a DAG, but geometry forces and projectors may depend on geometry from selected pass groups. The scheduler therefore uses explicit compute stages:

- Stage 0: bases, hero meshes, collision/reference geometry
- Stage 1: trunks and primary structures
- Stage 2: descendants and vines that consume Stage 1 geometry
- Stage 3: leaves, details, decals, projectors
- Stage 4: global post processes

Users may assign a generator to a later stage when it must react to earlier geometry. Cycles between stages are rejected. Iterative solvers are internal to one stage and have deterministic iteration limits.

## Generator contract

```cpp
struct GeneratorDescriptor {
    TypeId type;
    std::string_view display_name;
    std::span<const PropertyDescriptor> properties;
    std::span<const TypeId> allowed_parents;
    std::span<const TypeId> allowed_children;
    ComputeStage default_stage;
    CapabilityFlags capabilities;
};

class IGeneratorEvaluator {
public:
    virtual ~IGeneratorEvaluator() = default;
    virtual Result<CandidateSet> generate_candidates(
        const EvaluationContext& context,
        const GeneratorInstance& generator,
        const ParentOutput& parent) const = 0;

    virtual Result<GeneratorOutput> evaluate(
        const EvaluationContext& context,
        const GeneratorInstance& generator,
        const CandidateSet& candidates) const = 0;
};
```

Public interfaces use owned or scoped views with explicit lifetimes. Plugin ABI wrappers expose equivalent C handles.

## Candidate model

A candidate describes a potential generated node before expensive geometry exists:

- Semantic ID seed material
- Parent semantic ID and attachment coordinate
- Local frame
- Generation-mode ordinal and group
- Preliminary length, radius, scale, and material choice
- Random stream keys
- Visibility and pruning state

This separation lets the system prune, collide, or remap candidates before mesh generation.

## Generation modes

### Interval

Place nodes at a target spacing along the eligible parent interval. Include phase offset, jitter, start/end clipping, and optional endpoint placement.

### Phyllotaxy

Place groups using divergence angle, members per whorl, axial spacing, twist, and orientation controls. The same mode can place leaves, twigs, or decorations.

### Bifurcation

Recursively split a spine using threshold, probability, angle, asymmetry, and depth controls.

### Proportional and proportional steps

Choose count from parent length and density, either continuously or in steps. Preserve stable ordinals when parent length changes within a step.

### Absolute and absolute steps

Place an explicit count, either exact or selected from step rules.

### Classic

Provide legacy-style branch distribution with frequency, first/last, spread, and orientation behavior suitable for conventional trees.

### Flood

Populate a surface, zone, or parent area using density, exclusion radius, masks, and deterministic blue-noise-like sampling.

### Parent

Create one node derived directly from each eligible parent node.

## Stable random service

```text
stream_key = hash(
  document_seed,
  generator_id,
  semantic_parent_id,
  property_key,
  purpose_tag,
  algorithm_version)
```

A counter-based or splittable PRNG is preferred. Each decision consumes a named counter so adding a later decision does not shift earlier values.

## Dirty propagation

Property descriptors declare invalidation categories:

- UI only
- Material only
- Transform/placement
- Spine
- Local geometry
- Descendants
- Force stage
- Global post process
- LOD only
- Export only

The evaluator computes a minimal affected subgraph and reuses content-addressed results from unchanged generators.

## Caching

Cache keys include:

- Document algorithm set and schema version
- Generator type/plugin version
- Resolved properties
- Parent output hash
- Relevant asset hashes
- Target resolution and feature profile
- Timeline sample

Cache entries are immutable and may be stored in memory or on disk. Production jobs may share a remote content-addressed cache.

## Diagnostics

Every generator output includes:

- Node and triangle counts
- Compute time by phase
- Memory use
- Bounding boxes
- Warnings such as self-intersection, missing material, degenerate UV, orphaned edit, unsupported export feature
- Provenance from output semantic element to generator and edit layer

## Cancellation and progress

Each job has a cancellation token and hierarchical progress tree. Algorithms must check cancellation at bounded intervals. A cancelled evaluation never replaces the last valid snapshot.

## Deterministic parallelism

Parallel work partitions by stable semantic ranges. Results are gathered and sorted by semantic ID before final assembly. Floating-point reductions use a fixed reduction tree or compensated summation when order matters.

## Failure containment

A generator failure produces a diagnostic and an empty or last-known-good output according to policy. It must not corrupt unrelated cached outputs. Plugin crashes should be containable through the out-of-process host mode.

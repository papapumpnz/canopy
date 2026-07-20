# Manual editing and art-direction tools

## Editing model

Manual editing never mutates generated source data directly. Tools create transactions containing node overrides or edit-layer operations. The evaluator replays these operations after the relevant procedural phase.

## Selection

Supported selection domains:

- Generator
- Generated node
- Spine control point or segment
- Mesh vertex, edge, face, or region
- Leaf/frond instance
- Material or texture region
- Scene helper, force, target, camera, or light

Selection queries store semantic IDs plus spatial fallback signatures. Picking uses GPU ID buffers with CPU verification.

## Generator mode

Edits apply to a generator definition and affect all its output. The property panel shows base values, variance, parent/profile curves, rules, animation, and inherited defaults.

## Node mode

Edits apply only to selected generated nodes. Values are offsets or replacements according to property metadata. The UI displays the resolved procedural value and the override separately.

## Freehand mode

Freehand tools operate directly in the viewport with brush or spline input. All strokes are resampled to a deterministic object-space representation before commit.

## Hand draw

Workflow:

1. Select a parent surface, target, or world plane.
2. Draw a stroke or place spline points.
3. Fit a smooth spine with configurable tolerance.
4. Choose attachment and orientation rules.
5. Create a hand-drawn Branch, Spine, or Vine generator.
6. Edit control points, tangents, roll, radius, and constraints.

Procedural branches may be converted to hand-drawn spines while retaining descendants and semantic ancestry.

## Click place

Places a branch, leaf, mesh detail, target, or helper at a clicked location. Options:

- Surface, spine, grid, or world snapping
- Normal, tangent, world-up, camera, or custom orientation
- Parent assignment
- Randomized transform using a named stream
- Repeated placement and paint scattering
- Duplicate suppression radius

## Bend tool

The bend tool creates a deformation field over a spine interval or selected mesh region. It supports move, rotate, arc, smooth, straighten, and relax modes. Falloff is expressed in physical or normalized distance.

## Displacement paint

Displacement sources may be noise, texture, procedural field, or painted scalar. Brush types:

- Airbrush
- Add/subtract
- Smooth
- Flatten
- Fill
- Erase

The authoritative stroke stores surface-space samples and reconstructs values on regenerated topology through semantic surface coordinates.

## Trim tool

Trim modes:

- Cut at brush boundary
- Shrink branch length toward the base
- Grow branch length within original procedural limits
- Prune entire branch or subtree
- Remove selected leaf/frond/detail nodes
- Shape silhouette by shortening branches that cross a screen-space stroke
- Restore selected or all trim operations

A screen-space stroke is converted into camera metadata plus world-space hit intervals. The final committed operation must remain stable after viewport resolution changes.

Trimmed branches regenerate correct caps, descendants, wind anchors, growth timing, and all LODs.

## Vertex edit

Vertex edit creates local deformation cages or sparse vertex deltas. It supports:

- Move along world, local, normal, or view axes
- Soft selection
- Smooth and relax
- Preserve volume
- Preserve boundary or seam
- Symmetry
- Retarget to regenerated topology through semantic coordinates

For hero meshes, vertex edit can assign wind anchors and bone weights.

## Vertex colors and features

The user may paint:

- Standard color sets
- Scalar feature channels
- Vector channels
- Wind masks
- Blend weights
- LOD retention weights
- Custom studio attributes

Channels have names, types, ranges, defaults, export semantics, and color-space metadata where relevant.

## Shape control

Shape control alters branch distribution and orientation through high-level envelopes and targets:

- Conical, spherical, cylindrical, planar, custom spline, or mesh envelope
- Attract or repel from target points/curves
- Crown flattening and side trimming
- Directional growth preference
- Prune branches outside or inside the envelope

The control produces property and pruning contributions rather than baking arbitrary transforms where possible.

## Mesh helpers

Mesh helpers support hero-mesh rigging:

1. Place ordered markers on a mesh.
2. Compute a surface-following or interior guide curve.
3. Create a spine-only branch from the curve.
4. Build a hierarchy of helper spines.
5. Bind mesh vertices to one or more spines or bones.
6. Grow procedural descendants from named anchors.
7. Preview wind and export skinning or vertex wind data.

## Art-director gizmos

High-frequency properties can expose direct manipulators in the viewport:

- Length and radius
- Start/end region
- Force strength and direction
- Envelope size
- Target direction
- Wind response
- LOD thresholds

Gizmo changes execute the same property commands as the property panel.

## Undo and command model

```cpp
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual Result<void> apply(DocumentTransaction&) = 0;
    virtual Result<void> revert(DocumentTransaction&) = 0;
    virtual CommandDescription describe() const = 0;
    virtual bool can_merge_with(const ICommand&) const = 0;
};
```

Brush moves are coalesced into one command per stroke. Long computations are consequences of commands and are not themselves stored in undo history.

## Orphan handling

Regeneration can invalidate node edits. The modeler must show:

- Exact remaps
- Probable remaps requiring confirmation
- Orphaned operations
- Operations excluded by current resolution or season

Users can rebind, disable, delete, or convert orphaned edits into generator-level changes.

## Acceptance

- Every tool round-trips through save/load and undo/redo
- Editing unrelated generators does not change manual targets
- Trim remains valid across LOD and growth samples
- Brush results are stable across DPI and viewport size
- Procedural-to-hand-drawn conversion preserves descendants
- Mesh-helper rig remains valid after material-only changes

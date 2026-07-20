# Wind, growth, seasons, and timeline

## Unified animation model

Project Canopy stores semantic motion data independently from a target representation. The same authored model can generate:

- GPU vertex wind for real-time use
- Bone-based wind
- Static-topology point or vertex caches
- USD or Alembic time samples
- Topology-changing growth caches
- Still seasonal states

## Fan and wind field

A scene wind object contains:

- Algorithm profile: game, VFX, or integration compatibility
- Direction and strength
- Gust frequency, duration, rise/fall, and variance
- Turbulence scale and speed
- Branch, leaf, frond, and vine response groups
- Timeline source: current value, animated track, or external runtime input
- Preview quality and deterministic sampling seed

Multiple wind sources may be blended in engine integrations, while the authoring fan provides a canonical tuning reference.

## Game wind

### Semantic vertex data

Authoring computes, but does not pre-pack:

- Primary anchor and offset
- Secondary anchor or parent anchor
- Hierarchy level
- Normalized branch position
- Bend stiffness
- Branch phase
- Leaf/frond ripple mask
- Leaf tumble/twitch parameters
- Geometry type
- Wind group
- Optional bone indices/weights

### Shader motion

A reference game shader combines:

1. Low-frequency whole-tree sway
2. Hierarchical branch bending
3. Directional response
4. Gust modulation
5. High-frequency leaf/frond ripple
6. Optional leaf tumble or twitch
7. LOD-safe amplitude scaling

The shader must preserve the trunk base and support object rotation and nonuniform direction input. Quality tiers disable expensive terms without requiring re-export when packed data is present.

### CPU state

Runtime wind state advances deterministic oscillators and gust envelopes, then uploads a compact constant block. Engine adapters may replace this with native systems while retaining equivalent semantics.

## VFX wind

VFX wind prioritizes quality and directability:

- Hierarchical branch dynamics
- Per-level stiffness, damping, inertia, and phase
- Leaf/frond flutter and chaos
- Gusts and directional changes
- Optional collision for hero vines or branches
- Timeline curves and frame-rate-independent sampling

Export modes:

- Skeleton and skinning
- Static-topology point cache
- Alembic geometry cache
- USD time samples
- Baked per-frame meshes for diagnostic use

## Wind rigging for hero meshes

Mesh-helper spines define a bone hierarchy. Vertices are assigned weights by geodesic or Euclidean distance, artist masks, and attachment regions. The modeler visualizes weights and provides normalize, smooth, flood, and erase tools.

## Growth

Growth is evaluated from a normalized lifecycle time plus timeline speed.

Supported generators include Branch, Leaf Mesh, Frond, Fin, Cap, Knot, and compatible plugin generators. Batched leaves may convert to individual leaves for topology-changing growth.

Per-generator controls:

- Start offset and duration
- Speed multiplier and curve
- Parent-relative or in-place timing
- Length, radius, and scale development
- Emergence angle and orientation transition
- Wobble, gravity, and settling
- Material or color transition
- Descendant offset
- Visibility threshold

### Parent timing

A child begins when the parent reaches the child attachment coordinate plus configured offset. The attachment itself moves as the parent grows.

### In-place timing

A child remains attached to its final world-relative parent location and starts only when growth reaches that location. This avoids artifacts for vines or geometry wrapped around objects.

### Growth wizard

The wizard analyzes the hierarchy and initializes a coherent growth setup:

- **Grow** profile: parent-relative organic development
- **Reveal** profile: in-place tracing for constrained branches and vines
- Conversion warnings for unsupported batched representations
- Suggested end frame and speed curve based on hierarchy depth

It writes ordinary properties and can be undone.

### Growth export

Topology-changing growth must use USD or Alembic time-sampled geometry. Static-topology approximations may use scale and bones but are labeled as approximations.

## Seasons

A document season value normally ranges from `0` to `1`. Each eligible node computes a lifecycle transition value from:

- Global season value
- Start offset
- Time scale
- Variance
- Parent or descendant offset

The transition value drives:

- Weighted material selection curves
- Visibility and leaf drop
- Size
- Branch or frond gravity/droop
- Curl and fold
- Color and custom rule parameters
- Mesh detail and decal appearance

Season evaluation is per node so transitions remain mottled and natural.

## Dropped foliage

When a leaf or frond passes its drop threshold:

1. Compute detachment transform.
2. Sample deterministic fall parameters.
3. Intersect the ground plane or selected mesh force.
4. Settle with orientation and overlap policy.
5. Disable normal tree wind by default.
6. Emit to a ground-debris batch with source metadata.

## Continuous seasonal animation extension

For parity, single-state seasonal export is mandatory. Project Canopy may additionally animate continuous season changes. This extension must preserve the single-state workflow and clearly distinguish topology-changing transitions from shader-only color changes.

## Timeline

Timeline features:

- Start/end frame and frame rate
- Growth speed track
- Wind strength/direction/gust tracks
- Season track
- Camera and light tracks
- Rule parameter tracks
- Loop and playback range
- Draft/production preview modes
- Deterministic frame sampling

## Animation caching

Cache keys include document revision, animation profile, frame, sample rate, and target representation. Adjacent frames may share immutable topology and material data.

## Acceptance

- Game wind data survives custom packing and engine import
- Rotated instances respond to world wind correctly
- Wind quality tiers reduce cost without visible discontinuity
- Hero mesh bones deform continuously across stitch boundaries
- Growth parent and in-place reference scenes match expected timing
- Growth USD/Alembic exports contain correct topology per frame
- Seasonal material probabilities and dropped leaves are deterministic
- Leaves that have dropped do not use tree wind unless debris wind is enabled

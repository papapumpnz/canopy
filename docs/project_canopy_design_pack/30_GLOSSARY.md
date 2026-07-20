# Glossary

## A

**Absolute parent curve** — A parent-dependent curve evaluated against physical distance along the full parent rather than a normalized local interval.

**Anchor** — A stable attachment record on a curve or surface, including identity, local coordinates, orientation and rebind information.

**Art Director control** — A high-level spatial or semantic control that modifies many generated elements through one non-destructive operation.

**Authoring asset** — Editable source data such as a project, material, mesh, texture, preset or rule.

## B

**Base asset** — One immutable runtime vegetation model referenced by many instances.

**Batched leaf** — A generator/output strategy that groups many leaf elements into efficient geometry while retaining placement and variation semantics.

**Billboard** — A low-cost planar representation that changes orientation or image selection relative to the camera.

**Bifurcation** — A generation mode or event that divides a spine into two or more child directions.

**Boundaries** — The first and last usable region on a parent used for placement or curve evaluation.

**Branch anchor** — Data that identifies a point, frame and hierarchy position on a branch for wind, attachment or export.

**Branch frame** — Orthonormal orientation transported along a branch spine and used to construct cross sections and place children.

## C

**Cage** — A lower-resolution control mesh used to deform or constrain another mesh.

**Canonical serialization** — A stable representation with deterministic ordering, formatting and numeric rules, suitable for hashing and diffs.

**Cap** — Geometry that closes or decorates an open branch end or cut.

**Card** — A planar or near-planar textured geometry element used for leaves, fronds, clusters or low-detail vegetation.

**Cell** — A spatial population unit used for forest streaming and culling.

**Clean room** — An implementation process designed to derive independent software from approved requirements without protected implementation material.

**Click place** — A manual tool that adds an element at a selected curve or surface point with controlled orientation and snapping.

**Collision object** — Simplified geometry used by physics, placement, vines, pruning or export rather than primary rendering.

**Content hash** — A digest derived from canonical content and dependencies, used for cache identity and integrity.

**Continuous LOD** — A smoothly varying detail representation controlled by one normalized value rather than only discrete meshes.

**Curve profile** — A one-dimensional function that modulates a property along a parent, spine, age, season or other domain.

**Cutout mesh** — Polygonal geometry derived from a texture region or alpha silhouette.

## D

**Data packing** — Mapping semantic vertex, mesh or texture data into target channels and layouts.

**Decal** — Surface-following detail geometry or material used for bark features, scars, moss and similar localized effects.

**Dependency fingerprint** — Stable hash of every input that can affect an evaluation stage.

**Determinism** — The property that equivalent declared inputs produce equivalent declared outputs independent of irrelevant process conditions.

**Discrete LOD** — One of a finite set of baked meshes or representations chosen at runtime.

**Draw packet** — Renderer-neutral description of geometry, material, instance range, LOD and flags needed for a draw.

**Dropped foliage** — Leaf, frond or detail elements detached during a seasonal or growth state and optionally deposited on a surface.

## E

**Edit layer** — Non-destructive manual modifications applied over generated data and keyed to stable entities.

**Evaluation profile** — Named quality, product and resource settings for draft, preview, production, export or runtime compilation.

**Evaluation snapshot** — Immutable, revision-tagged result of evaluating a document.

**Export manifest** — Machine-readable record of source, preset, versions, outputs, hashes, statistics and warnings.

**ExportScene** — Normalized, immutable intermediate scene consumed by all exporters.

## F

**Feature vertex** — A mesh vertex or annotated sample retained because it represents a visually or structurally important feature.

**Fin** — Thin geometry protruding from a surface, often used for bark strips, moss, needles or silhouette detail.

**Forest** — A population of instances plus spatial, wind, LOD, culling and streaming state.

**Frond** — Ribbon, blade or compound planar vegetation geometry such as palm leaves or ferns.

**Freehand mode** — Direct viewport editing mode for bend, displacement, trim, drawing, painting and vertex manipulation.

## G

**Generation mode** — Placement strategy controlling how child nodes are created on or around a parent.

**Generator** — A typed procedural graph object that creates nodes, geometry, modifiers, helpers or outputs.

**Generator DAG** — Directed acyclic graph of generator relationships, allowing validated reuse while preventing dependency cycles.

**Geometry class** — Semantic category such as branch, leaf, frond, card, detail, proxy, billboard or collision.

**Geometry force** — Spatial field or object that attracts, repels, aligns, obstructs, prunes, stops or constrains generated curves/elements.

**Golden asset** — Original test model with reviewed expected topology, statistics, semantics and reference renders.

**Growth age** — Normalized or physical value representing plant development on the growth timeline.

## H

**Hand-drawn generator** — Generator whose principal spines or placements are created and edited manually rather than regenerated from ordinary placement rules.

**Hero mesh** — High-value imported mesh, often sculpted or scanned, used as a primary plant component and optionally rigged with Canopy spines.

**Hysteresis** — Different enter and exit thresholds used to prevent LOD or state oscillation near a boundary.

## I

**Impostor** — Multi-view image-based representation that can reconstruct orientation, normal and sometimes depth for distant rendering.

**Incremental evaluation** — Recomputing only stages invalidated by a document change.

**Instance** — Placement and variation state referencing shared base asset data.

**Internode** — Segment of a plant axis between successive nodes or branching points.

## J

**Junction** — Region where a child branch connects to its parent and mesh topology/normals must transition.

## K

**Knot generator** — Detail generator representing knots, burls, scars or comparable branch-surface features.

## L

**Leaf mesh** — Individual leaf geometry, often a cutout or imported mesh, placed procedurally or manually.

**Lightmap UV** — Unique nonoverlapping UV set intended for baked lighting.

**Lockfile** — Project record pinning exact asset package versions and hashes.

**LOD remap** — Mapping from source elements to survivors or replacements in another detail level.

## M

**Material set** — Named group of weighted or state-dependent materials selectable by element, season or variation.

**Mesh converter** — Generator/workflow that derives procedural spine, cross-section, texture or attachment information from an imported mesh.

**Mesh detail** — Surface-attached geometry created from a mesh/material pattern and blended or wrapped to a parent.

**Mesh helper** — Marker and curve tooling used to extract or assign spine hierarchies on a hero mesh.

**Meshlet** — Small bounded cluster of mesh primitives suitable for GPU culling and rendering.

**Metadata** — Structured user or pipeline data attached to a document, asset, generator, node or runtime model.

## N

**Named random stream** — Deterministic pseudorandom sequence keyed by semantic purpose rather than call order.

**Node** — A generated anatomical or geometric instance created by a generator.

**Node override** — Per-node property or transform modification layered over generator output.

## O

**Origin rebasing** — Shifting the floating-point coordinate origin while preserving large-world positions and stable identities.

**Out-of-process plugin** — Extension executed in a separate process with bounded resources and a versioned protocol.

**Overdraw** — Repeated shading of pixels, especially common in overlapping alpha-tested foliage.

## P

**Package tree** — Saving a project together with selected dependent assets into a transferable package.

**Parallel transport** — Method for moving an orientation frame along a curve with minimal unintended twist.

**Parity evidence** — Test, benchmark, workflow recording or artifact demonstrating an equivalent capability.

**Phenology** — Seasonal biological timing such as leaf emergence, color change and drop.

**Photogrammetry** — Reconstruction of meshes and textures from photographs; in this product, also the workflow for extending and converting scanned vegetation.

**Phyllotaxy** — Arrangement of leaves or branches around a stem, commonly expressed by angular divergence and spacing.

**Point cache** — Time-sampled per-point geometry animation, typically exchanged through Alembic or USD.

**Population source** — Provider of instance placements consumed by the forest system.

**Procedural graph** — Network of generators and dependencies evaluated to produce plant anatomy and geometry.

**Projector** — Scene object that projects texture, attributes or masks onto target geometry.

**Proxy** — Simplified helper or output representation used for selection, collision, LOD or other nonhero purposes.

## R

**Reference generator** — Reusable placeholder or indirection node that instantiates a shared generator subhierarchy.

**Relative parent curve** — A parent-dependent curve evaluated against normalized position within a usable parent interval.

**Resolution** — Authoring quality control that changes sampling and geometry density without changing the intended plant design.

**Rule** — Sandboxed Lua script exposing high-level controls and mapping them to documented model properties.

**Runtime asset** — Compiled `.canopyrt` model intended for fast loading and forest rendering.

## S

**Season channel** — Curve or state controlling a property over a seasonal value.

**Section directory** — Table of typed, ranged and checksummed blocks in a runtime file.

**Shade pruning** — Removal or reduction of foliage based on estimated light exposure or its inverse.

**Shell** — Surface layer offset from a parent, used for bark, moss, snow or similar coverage.

**Silhouette error** — Difference between projected outlines used to evaluate LOD or optimization quality.

**Snapshot revision** — Document revision from which an immutable evaluation result was produced.

**Spine** — Ordered curve and frames describing the central axis of a branch, vine, frond or rigged mesh region.

**Spine-only branch** — Branch entity carrying hierarchy, deformation and attachment data without generating its own visible tube geometry.

**Stable identity** — Identifier derived independently of evaluation order and preserved through valid regeneration.

**Stitch** — Geometry and shading transition joining imported and procedural components.

**Subsurface/transmission** — Approximation of light passing through thin leaves or plant tissue.

## T

**Target generator** — Precise marker used to locate child generation on a parent, often on imported mesh surfaces.

**Texel density** — Texture pixels per world-space unit.

**Texture atlas** — Combined image containing regions for multiple materials or elements.

**Timeline** — Time control for wind, growth, seasons and exported animation.

**Topology-stable animation** — Animation in which vertex/index connectivity remains fixed.

**Topology-varying animation** — Animation in which point or face counts/connectivity change over time.

**Transaction** — Atomic set of document mutations that commits or rolls back as one unit.

**Trim** — Manual edit that cuts, shrinks, grows or removes branch regions.

## U

**UDIM** — Tiled UV convention commonly used for high-resolution VFX texturing.

**Undo journal** — Durable or in-memory command history that restores prior committed document states.

**Unit dimension** — Semantic measurement category such as length, angle, time or unitless ratio.

**UV area** — Named texture region with transform, pivot and optional anchor metadata.

## V

**Variation recipe** — Deterministic distributions, correlations and constraints used to create related plant variants.

**Vertex feature** — Named scalar/vector attribute painted or generated for shading, wind, masking or export.

**Vine** — Curve-based plant element solved against gravity, guides, geometry and constraints, with optional low-poly representation.

## W

**Wind domain** — Spatial or logical wind state shared by a set of instances.

**Wind hierarchy** — Branch-level grouping and weights controlling coherent plant deformation.

**Wind profile** — Authoring and runtime parameters describing quality tier, response, gusts and motion components.

**Workflow parity** — Ability to complete equivalent classes of task and produce equivalent practical outputs without binary compatibility.

## Z

**Zone generator** — Generator that creates or constrains elements within a defined volume or surface region.

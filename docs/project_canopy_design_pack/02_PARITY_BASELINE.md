# Functional parity baseline

## Baseline

The comparison target is the public SpeedTree product and documentation available on 2026-07-19, including SpeedTree Modeler 10.2.0, the Modeler 10 documentation map, game and VFX exports, and Runtime SDK 10 documentation.

Parity is evaluated by workflow and output, not by copying implementation details or proprietary formats.

## Parity matrix

| Area | Required capability | Project Canopy equivalence | Gate |
|---|---|---|---|
| Product | Unified games and VFX modeler | One desktop application and one document model | GA |
| Modeling | Procedural generator hierarchy | Typed DAG with anatomical parent-child semantics | Alpha |
| Modeling | Generator and node editing modes | Generator properties plus per-instance edit layers | Alpha |
| Modeling | Curves, variance, random seeds | Versioned curve model, variance distributions, named random streams | Alpha |
| Modeling | Generation modes | Interval, phyllotaxy, bifurcation, proportional, stepped, absolute, classic, flood, parent | Beta |
| Modeling | Tree, branch, leaf, batched leaf, frond | Native equivalent generators | Beta |
| Modeling | Zone, vine, card, base, reference | Native equivalent generators | Beta |
| Modeling | Cap, decal, fin, knot, shell | Native equivalent detail generators | Beta |
| Modeling | Mesh, mesh converter, mesh detail, stitch | Imported-mesh and conversion generators | Beta |
| Modeling | Target, spine, subdivision, cage, proxy | Helper and legacy-equivalent generators | Beta |
| Modeling | Randomization and variants | Seeded deterministic variants with constraints | Alpha |
| Modeling | Leaf collision and shade pruning | Post-evaluation collision and light-exposure pruning | Beta |
| Modeling | Mesh anchors and clusters | Named surface anchors and grouped placements | Beta |
| Modeling | Splits and branch shape control | Spine split rules and envelope/attractor shaping | Beta |
| Modeling | Geometry forces | Attract, avoid, obstruct, crawl, prune, stop, align, ground and mesh forces | Beta |
| Modeling | UV tiling and patching | UV regions, tiled bark, patches, blending, and density controls | Beta |
| Manual editing | Bend and displacement tools | Brush and gizmo edit layers | Beta |
| Manual editing | Trim brush | Cut, shrink, grow, prune, remove, selective reset | Beta |
| Manual editing | Hand-drawn branches | Editable spline drawing and procedural-to-drawn conversion | Beta |
| Manual editing | Click place | Surface-aware placement with snapping and orientation | Beta |
| Manual editing | Vertex edit | Local vertex deformation with topology-preserving constraints | Beta |
| Manual editing | Vertex features and colors | Paintable named scalar/vector/color channels | Beta |
| Manual editing | Mesh helpers and hero-mesh spines | Surface markers, curve extraction, hierarchy rigging | Beta |
| Materials | PBR materials and maps | Metal/rough or spec/gloss profiles, subsurface, opacity, normals, displacement | Alpha |
| Materials | Material sets | Named weighted sets and variants | Alpha |
| Materials | Cutout editor | 2D polygon editor, triangulation, alpha cleanup, pivots, anchors | Beta |
| Materials | UV area editor and atlases | Region editor and atlas authoring | Beta |
| Materials | Texture adjustments and packing | Non-destructive operations and channel packing | Beta |
| Photogrammetry | Scan import and texture creation | Mesh/image ingest, map generation hooks, cleanup workflow | Beta |
| Photogrammetry | Procedural conversion and feature vertices | Surface/spine extraction and high-value feature retention | Beta |
| Photogrammetry | Bake stitch, texture blend, vertex blend | Three explicit connection strategies | Beta |
| Photogrammetry | Branch/twig preparation and placement | Pivot, anchors, attachment sites, orientation metadata | Beta |
| Vines | Physics-based hanging and crawling | Deterministic constrained-rod/PBD authoring solve | Beta |
| Vines | Manual placement, guide following, low-poly mode | Native workflows and profile presets | Beta |
| Animation | Game wind | GPU vertex wind with quality tiers and gusts | Beta |
| Animation | VFX wind | Bone and cache output with high-frequency leaf motion | Beta |
| Animation | Legacy integration profiles | Compatibility profiles implemented in engine adapters | RC |
| Animation | Growth | Timeline, generator growth parameters, parent/in-place timing | Beta |
| Animation | Growth wizard | Rule-based automatic timing initialization | Beta |
| Animation | Seasons | Node lifecycle, material curves, size, gravity, curl, fold, drop | Beta |
| Animation | Dropped leaves | Ground or mesh deposition with wind disabled after drop by default | Beta |
| Rendering | PBR viewport | HDR environment, direct light, shadows, AO, subsurface approximation | Alpha |
| Rendering | Render modes | Configurable debug and beauty passes | Beta |
| Rendering | Resolution controls | Draft through production geometry profiles | Alpha |
| Rendering | Cameras and image render | Named cameras, turntables, still and sequence rendering | Beta |
| Optimization | Automatic dynamic LOD | Continuous authoring LOD and baked runtime levels | Beta |
| Optimization | Smooth part removal | Spine shrink, leaf shrink/grow compensation, dither/crossfade | Beta |
| Optimization | Billboards | Multi-view impostor atlas with material passes | Beta |
| Optimization | Collision objects | Generated primitives, convex meshes, custom proxies | Beta |
| Optimization | Per-vertex AO | Baked vertex AO and texture AO options | Beta |
| Export | Games presets | Unity, Unreal, Godot, generic runtime and one-draw-call profiles | Beta |
| Export | VFX presets | Maya, 3ds Max, Houdini, Blender, Cinema 4D, generic profiles | Beta |
| Export | FBX, OBJ, USD, Alembic, XML-like raw output | Equivalent exports; FBX through optional Autodesk SDK adapter | Beta |
| Export | Static, bones, point cache, topology-changing cache | Static mesh, skinning, USD/Alembic time samples | Beta |
| Export | Texture atlases and billboards | Deterministic packer and impostor baker | Beta |
| Export | Lightmap UVs | Optional unique secondary UV generation | Beta |
| Export | Custom mesh/vertex packing | Sandboxed Lua packing scripts | Beta |
| Export | Command-line export | Full headless CLI with preset files | Alpha |
| Export | Custom exporters and pipeline SDK | Public plugin SDK and Python API | RC |
| Export | Package tree with assets | `.canopy` package and unpacked `.canopyproj` form | Alpha |
| Scripting | Rules UI and property control | Sandboxed Lua rule panels | Beta |
| Library | Production-ready editable asset catalog | Catalog system plus original content packs | RC |
| Runtime | Compact runtime file | `.canopyrt` memory-mappable package | Beta |
| Runtime | Geometry, materials, textures, bones, billboards | Versioned model resource API | Beta |
| Runtime | Wind management | Per-species and forest-level wind state | Beta |
| Runtime | Cell-based culling and LOD | Grid/clipmap cells, instance culling, screen-error LOD | Beta |
| Runtime | Millions of instances | GPU-driven indirect rendering path | RC |
| Runtime | Cascaded shadow support | Reference implementation and engine hooks | RC |
| Runtime | Allocation and file callbacks | C API callback tables | Beta |
| Runtime | Coordinate conversion and metadata | Explicit axis/unit conversion and key-value metadata | Beta |
| Runtime | Collision access | Runtime collision object API | Beta |
| Runtime | Optional terrain cells and LOD | Reference heightfield streamer with crack-free tile LOD | RC |
| Integrations | Unity and Unreal | Importers, materials, wind, LOD, billboards, terrain placement | RC |
| Integrations | Major DCC applications | Import helpers and material reconstruction | RC |

## Release terminology

- **Alpha**: feature is usable in the headless core and has deterministic tests
- **Beta**: complete artist workflow exists in the modeler and export path
- **RC**: integrations, performance, compatibility, and documentation are complete
- **GA**: every acceptance item in `32_ACCEPTANCE_CHECKLIST.md` is signed off

## Compatibility statement

Project Canopy must never label proprietary SpeedTree formats as supported unless support is delivered under an explicit license. Public marketing should use “functional alternative” or “workflow parity,” not “drop-in binary replacement.”

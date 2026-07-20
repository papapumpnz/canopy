# Viewport and editor user experience

## Main-window layout

Default docks:

- Central Tree/Scene Viewport
- Generation Graph
- Property Bar
- Assets browser with Materials, Material Sets, Meshes, Displacements, and Masks
- Timeline
- Rules panel
- Messages and diagnostics
- Statistics/profiler

Every dock can be moved, tabbed, resized, floated, hidden, and restored. Named layouts support Modeling, Materials, Photogrammetry, Animation, Game Optimization, VFX, and Scripting.

## Viewport rendering

The modeler viewport uses WebGPU through a narrow renderer abstraction. Required features:

- HDR PBR rendering
- Directional and ambient/environment lighting
- Shadow maps with configurable quality
- Screen-space or ground-truth-preview AO
- Alpha-tested and two-sided foliage
- Subsurface/transmission approximation
- Displacement preview within target limits
- Wind, growth, seasons, and LOD preview
- Selection outline and wireframe
- GPU picking
- High-DPI rendering and screenshot output

## Render modes

Built-in modes:

- Beauty/PBR
- Base color, opacity, normal, roughness, metallic, AO, subsurface, height
- Diffuse and specular lighting
- Flat and low-poly
- Wireframe and topology
- Normals/tangents/binormals
- UV density and seams
- Vertex colors and named feature channels
- Wind groups, anchors, stiffness, hierarchy
- Growth time and season lifecycle
- LOD importance and morph state
- Overdraw and shade exposure
- Collision, bones, proxies, cages, targets, and force fields
- Depth, object ID, semantic ID, and material ID
- Snow accumulation preview
- Albedo-value validation

Render modes are data-driven shader/pass configurations. Studios can add modes through a plugin or config without changing core UI.

## Scene objects

- Wind fan/field
- Directional light and environment
- Ground plane or mesh
- Geometry forces
- Targets and guides
- Cameras
- Collision/proxy meshes
- Mesh helpers

Viewport visibility filters do not modify export state.

## Navigation

- Orbit, pan, dolly, frame selection, frame all
- Configurable Maya, Blender, 3ds Max, and Canopy keymap presets
- Orthographic and perspective modes
- Named cameras and camera bookmarks
- Turntable and path preview
- Unit-aware grid and height indicator

## Generation graph

The graph presents the anatomical hierarchy, not a generic unrestricted node graph. Features:

- Add compatible child generator
- Drag to reparent with validation
- Duplicate, reference, convert, group, color tag, enable/disable
- Stage and force indicators
- Node counts, warning badges, compute cost
- Mini-hierarchy references
- Search and filter
- Focus/isolate selection
- Stable multi-selection and bulk property edits

## Property Bar

Properties are grouped by function and may display in multiple columns. Each row can expose:

- Value and units
- Variance
- Parent/profile curve buttons
- Rule/animation binding
- Reset and copy property path
- Node override state
- Validation status
- Art-director gizmo toggle

Mixed values and inherited values are visually distinct.

## Curve and variance editors

Curve editor requirements:

- Multiple curve overlays
- Tangent editing and interpolation modes
- Numeric point table
- Preset save/load
- Absolute/relative parent domain
- Histogram of actual node samples
- Copy/paste and normalization

Variance editor shows distribution, clamp, seed stream, and a live sample histogram.

## Cutout and UV editors

These are specialized 2D editors with zoom, pan, snapping, pixel grid, channel view, alpha threshold, triangulation preview, overdraw estimate, pivots, anchors, and multiple-cutout display.

## Timeline

- Playback, scrubbing, frame stepping, loop
- Growth, wind, season, camera, rule tracks
- Draft/production preview
- Cache status and invalidation display
- Current versus timeline wind indicator
- Frame and timecode display

## Background computation

The UI remains responsive. A task center shows hierarchical progress, cancellation, cache hits, and recent failures. Preview snapshots update progressively without selection loss.

## Messages

Messages are structured and filterable by severity, generator, asset, frame, exporter, and requirement ID. Selecting a message focuses the relevant object and property.

## Autosave and recovery

- Periodic journal and snapshot
- Recovery chooser after abnormal exit
- Side-by-side change summary
- Never replace explicit save without confirmation
- Preserve plugin extension data even when plugin is unavailable

## Accessibility

- Full keyboard navigation for core workflows
- Screen-reader labels for standard controls
- High-contrast and color-vision-safe indicators
- Scalable UI and icons
- Do not encode state by color alone
- Configurable motion reduction
- Text alternatives for viewport-only actions where feasible

## Preferences

- Units, axes, navigation, keymap
- Compute threads and cache location
- Viewport backend and quality
- Color-management config
- Autosave policy
- Plugin trust and isolation
- External tools
- Default export presets
- Network access for library services

## Onboarding

The New Model dialog includes original samples categorized by games, VFX, vines, hero meshes, growth, seasons, stylized assets, and optimization. It links to local documentation and does not require an online account for core use.

## Acceptance

- All core commands are available through menus and searchable command palette
- Layouts survive restart and monitor changes
- High-DPI behavior is correct on mixed-scale displays
- Background recompute never blocks input longer than the UI latency budget
- Viewport state and selection survive snapshot swaps
- Each diagnostic can navigate to its source

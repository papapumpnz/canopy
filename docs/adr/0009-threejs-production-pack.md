# ADR-0009: three.js production pack — textures, wind channels, tangents, LOD bake, impostors

- **Status**: Accepted
- **Date**: 2026-07-21
- **Design authority**: `12_MATERIALS_TEXTURES_AND_PHOTOGRAMMETRY.md` (first slice), `13_WIND_GROWTH_AND_SEASONS.md` (game wind semantics), `14_LOD_BILLBOARDS_AND_OPTIMIZATION.md` (impostor groundwork), `16/18` export & integrations

## Context

The GLB path is functionally complete (verified in three.js's GLTFLoader) but
production renderers need texture maps, live wind, tangents, LOD wiring, and
far-distance impostors. All must fit the zero-network, zero-dependency build.

## Decision

1. **Materials 1.3.0 (additive)**: optional `textures {base_color, normal}`
   (project-relative URIs under `assets/textures/`) and `card_region
   [u0, v0, u1, v1]` — an atlas region that switches that material's leaves
   to two-triangle textured cards with `alphaMode: MASK` (cutoff 0.5).
   Polygon cutouts remain supported (and used) — cards are the game-budget
   path (oak drops ~104k → ~30k triangles). `Document` carries a
   non-serialized `project_root` so exporters can resolve asset URIs.
2. **GLB embedding**: referenced PNGs are embedded verbatim as image
   bufferViews (no decoding in C++); materials gain `baseColorTexture` /
   `normalTexture`. OBJ/MTL writes `map_Kd` and copies textures beside the
   artifact for the diagnostic renderer.
3. **Tangents**: per-vertex MikkTSpace-style UV-gradient tangents (VEC4,
   handedness in w) generated at merge; degenerate-UV triangles fall back to
   a normal-perpendicular basis.
4. **Wind vertex channels**: custom attributes `_WIND_ANCHOR` (VEC3: node
   sway pivot) and `_WIND_PARAMS` (VEC4: amplitude<sub>rad</sub>, phase,
   hierarchy depth, kind) — the same per-node oscillator parameters the
   authoring wind uses, so shader wind and baked wind agree.
   `integrations/threejs/canopy-wind.js` injects the reference motion
   (bend about the anchor from amplitude/phase + gust) into any material
   via onBeforeCompile.
5. **LOD bake**: gltf presets accept `"bake_lods": true` → one command
   writes `<base>.lod0/1/2.glb` plus a combined manifest with switch
   distances derived from the model radius (the same 8r/20r bands the CPU
   forest uses). `integrations/threejs/canopy-lod.js` assembles a
   `THREE.LOD` from that manifest, with an optional impostor as the last
   level. Mesh compression (meshopt/Draco) stays dependency-gated.
6. **Impostors + AO (diagnostic grade)**: `tools/showcase/make_impostors.py`
   bakes an RGBA sprite from the diagnostic renderer and builds a crossed-
   quad GLB impostor; documented as an approximation until the real `14`
   impostor pipeline (view-dependent atlases) lands. Baked AO ships as
   `COLOR_0` crown-depth darkening (cheap, deterministic), clearly labeled
   approximate.

## Consequences

- Sample projects gain generated texture assets (procedural PNGs, provenance
  clean) and switch broadleaf species to card foliage; the spore tree keeps
  polygon cutouts so both paths stay exercised.
- Texture *processing* (photogrammetry, atlassing tools, mip policy) remains
  the full `12` epic; this slice defines the data contract and transport.
- Custom `_WIND_*` attributes are namespaced per the glTF spec; engines that
  ignore them see a static tree — strictly additive.

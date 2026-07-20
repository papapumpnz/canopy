# Using Canopy trees in three.js

Canopy exports **glTF (GLB)** for runtime use and **OBJ** for diagnostics and
per-node addressing. Prefer GLB in three.js — one binary file, PBR materials,
loads with `GLTFLoader`.

## Recommended: GLB

```bash
canopy-cli export MyTree.canopyproj \
  --preset presets/gltf.json \
  --out public/assets/my-tree --json
```

yields `my-tree.glb` + `my-tree.manifest.json`.

```js
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

const manifest = await (await fetch('assets/my-tree.manifest.json')).json();
const glbBytes = await (await fetch('assets/my-tree.glb')).arrayBuffer();
// Integrity check against the pipeline manifest:
const digest = await crypto.subtle.digest('SHA-256', glbBytes);
const hex = [...new Uint8Array(digest)].map((b) => b.toString(16).padStart(2, '0')).join('');
if (hex !== manifest.glb_sha256) throw new Error('canopy asset integrity check failed');

const gltf = await new GLTFLoader().parseAsync(glbBytes, '');
scene.add(gltf.scene); // one mesh, one primitive per material, meters, Y-up
```

Notes on the GLB contents (ADR-0007/0009):

- Geometry is merged **per material** (`manifest.granularity ===
  "material"`); use the OBJ path when you need per-branch semantic IDs.
- Materials are PBR with **embedded textures** (bark albedo + normal map,
  alpha-masked leaf atlas), TANGENT accessors for normal mapping, and
  `doubleSided` foliage. Colors fall back to `baseColorFactor` when a
  material has no textures.
- Every vertex carries **wind channels** (`_wind_anchor`, `_wind_params`);
  see "Live wind" below.
- Seasonal exports bake the season-blended colors: export at `--season 0.85`
  for an autumn variant of the same document.

## Live wind

Canopy GLBs carry the authoring wind rig as vertex data. The drop-in module
`integrations/threejs/canopy-wind.js` injects the reference motion into any
material — live shader wind then matches `canopy-cli export --time N` frames:

```js
import { applyCanopyWind, canopyWindUniforms } from 'canopy/integrations/threejs/canopy-wind.js';
const wind = canopyWindUniforms({ strength: 0.6, directionDeg: 25, gust: 0.5 });
gltf.scene.traverse((o) => { if (o.isMesh) applyCanopyWind(o.material, wind); });
renderer.setAnimationLoop((t) => { wind.uTime.value = t / 1000; renderer.render(scene, camera); });
```

## LOD chains and impostors

Bake three LOD GLBs and a switch-distance manifest in one command:

```bash
canopy-cli export MyTree.canopyproj --preset presets/gltf-lods.json --out assets/my-tree
# → my-tree.lod0.glb / .lod1.glb / .lod2.glb + my-tree.lods.manifest.json
```

Assemble with `integrations/threejs/canopy-lod.js`:

```js
import { loadCanopyLOD } from 'canopy/integrations/threejs/canopy-lod.js';
const tree = await loadCanopyLOD('assets/my-tree.lods.manifest.json', {
  impostor: { url: 'assets/my-tree.impostor.glb', distance: 120 }, // optional
});
scene.add(tree);
```

Impostors (12-triangle crossed billboards with a baked sprite) come from
`tools/showcase/make_impostors.py`; use them as the far level of large
forests.

## Diagnostic path: OBJ

The OBJ export keeps one group per branch node (`sem_<semantic-id>`), which
survives art iteration and suits gameplay markup:

```bash
canopy-cli export MyTree.canopyproj \
  --preset presets/obj-diagnostic.json \
  --out public/assets/my-tree --json
```

which yields `my-tree.obj`, `my-tree.mtl`, `my-tree.manifest.json`.

## 1. Load the manifest first

The manifest is small and cache-friendly; use it to integrity-check and to
key your caches.

```js
const manifest = await (await fetch('assets/my-tree.manifest.json')).json();

// Optional but recommended: verify the OBJ you fetched is the one the
// pipeline produced (bytes → SHA-256 → compare with manifest.obj_sha256).
const objBytes = await (await fetch('assets/my-tree.obj')).arrayBuffer();
const digest = await crypto.subtle.digest('SHA-256', objBytes);
const hex = [...new Uint8Array(digest)]
  .map((b) => b.toString(16).padStart(2, '0')).join('');
if (hex !== manifest.obj_sha256) throw new Error('canopy asset integrity check failed');
```

## 2. Load the OBJ

```js
import * as THREE from 'three';
import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';

const text = new TextDecoder().decode(objBytes);
const tree = new OBJLoader().parse(text);
```

Canopy exports are Y-up, right-handed, meters — the same convention as
three.js, so no axis fix-up is required.

## 3. Materials

The bootstrap `.mtl` is a stub; assign your own PBR materials. Each child of
the loaded group is one branch node whose name encodes the stable semantic ID
(`sem_<16-hex>`), and `material.name` carries the Canopy material name:

```js
const bark = new THREE.MeshStandardMaterial({ color: 0x6b4f3a, roughness: 0.9 });
tree.traverse((child) => {
  if (child.isMesh) child.material = bark;
});
```

Semantic IDs are deterministic across re-exports of the same document, so you
can attach game data (hit zones, cut points, LOD policies) to `sem_*` names
and they survive art iteration that does not change the graph.

## 4. Instancing a forest

Never clone tree meshes per instance; merge each tree's nodes once and use
`InstancedMesh`:

```js
import { mergeGeometries } from 'three/addons/utils/BufferGeometryUtils.js';

const parts = [];
tree.traverse((c) => { if (c.isMesh) parts.push(c.geometry); });
const merged = mergeGeometries(parts);

const count = 500;
const forest = new THREE.InstancedMesh(merged, bark, count);
const m = new THREE.Matrix4();
for (let i = 0; i < count; i++) {
  m.makeRotationY(Math.random() * Math.PI * 2)
    .setPosition(Math.random() * 200 - 100, 0, Math.random() * 200 - 100);
  forest.setMatrixAt(i, m);
}
scene.add(forest);
```

(For production forests, drive positions/rotations from your own seeded PRNG
so placements are reproducible — the same discipline Canopy applies
internally.)

## 5. Build-pipeline pattern

- Treat `.canopyproj` as source (it is git-friendly canonical JSON) and the
  export as a build artifact.
- Cache key: `document_hash` (from `canopy-cli validate --json`) + preset
  file hash. Skip the export step when unchanged.
- Run `canopy-cli export ... --json` in CI; fail the build when `ok` is not
  `true`, and store `model_hash` next to the bundle for provenance.

## Roadmap notes

- **Texture processing**: sample textures are procedural; the full
  photogrammetry/atlas pipeline (`12_MATERIALS...`) brings scanned bark,
  mipmap policy and compressed formats (dependency-gated).
- **Impostors**: current billboards are diagnostic-grade; view-dependent
  impostor atlases arrive with `14_LOD`.
- **Compression**: meshopt/Draco is deliberately deferred pending dependency
  intake (ADR-0009).

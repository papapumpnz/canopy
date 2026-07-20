# Canopy → three.js integration modules

Drop-in ES modules (no build step, import beside your three.js code):

| Module | Purpose |
|---|---|
| `canopy-wind.js` | Live wind: injects the Canopy oscillator model into any material using the `_wind_anchor` / `_wind_params` vertex channels every Canopy GLB carries. Matches `canopy-cli --time` baked frames. |
| `canopy-lod.js` | Builds a `THREE.LOD` from a `*.lods.manifest.json` (written by `canopy-cli export` with a `"bake_lods": true` gltf preset), optionally ending in a billboard impostor. |

Quick start:

```js
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { applyCanopyWind, canopyWindUniforms } from './canopy-wind.js';
import { loadCanopyLOD } from './canopy-lod.js';

// Single tree with live wind
const gltf = await new GLTFLoader().loadAsync('assets/oak.glb');
const wind = canopyWindUniforms({ strength: 0.6, directionDeg: 25 });
gltf.scene.traverse((o) => { if (o.isMesh) applyCanopyWind(o.material, wind); });
scene.add(gltf.scene);
// per frame:
wind.uTime.value = clock.getElapsedTime();

// LOD chain with an impostor tail
const tree = await loadCanopyLOD('assets/oak.lods.manifest.json', {
  impostor: { url: 'assets/oak.impostor.glb', distance: 120 },
});
scene.add(tree);
```

See `docs/integrations/three_js.md` for the full pipeline (export commands,
manifest verification, instancing patterns) and ADR-0009 for the data
contract. Impostor GLBs are baked by `tools/showcase/make_impostors.py`
(diagnostic grade until the view-dependent impostor pipeline lands).

/**
 * Canopy LOD assembly for three.js (ADR-0009).
 *
 * `canopy-cli export --preset gltf-lods.json` writes `<base>.lod0/1/2.glb`
 * and `<base>.lods.manifest.json` with switch distances derived from the
 * model's bounds (the same 8r/20r bands the Canopy CPU forest uses). This
 * helper turns that set into a THREE.LOD, optionally ending in a billboard
 * impostor for far distances.
 *
 * Usage:
 *   import { loadCanopyLOD } from './canopy-lod.js';
 *   const lod = await loadCanopyLOD('assets/my-tree.lods.manifest.json', {
 *     impostor: { url: 'assets/my-tree.impostor.glb', distance: 120 }, // optional
 *   });
 *   scene.add(lod);
 */
import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

export async function loadCanopyLOD(manifestUrl, { impostor, loader } = {}) {
  const gltfLoader = loader ?? new GLTFLoader();
  const manifest = await (await fetch(manifestUrl)).json();
  if (manifest.format !== 'canopy-lod-manifest') {
    throw new Error('not a canopy LOD manifest: ' + manifestUrl);
  }
  const baseUrl = manifestUrl.slice(0, manifestUrl.lastIndexOf('/') + 1);
  const lod = new THREE.LOD();
  for (const entry of manifest.lods) {
    const gltf = await gltfLoader.loadAsync(baseUrl + entry.file);
    lod.addLevel(gltf.scene, entry.switch_distance);
  }
  if (impostor) {
    const gltf = await gltfLoader.loadAsync(impostor.url);
    lod.addLevel(gltf.scene, impostor.distance);
  }
  return lod;
}

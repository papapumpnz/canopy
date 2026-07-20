/**
 * Canopy wind for three.js (ADR-0009).
 *
 * Canopy GLBs carry per-vertex wind channels:
 *   _wind_anchor (vec3) — the node's sway pivot in model space
 *   _wind_params (vec4) — tip amplitude (radians), phase, bend length (m),
 *                         kind (0 branch, 1 frond, 2 foliage). Foliage
 *                         vertices carry their parent branch's pivot and
 *                         parameters, so leaves bend with their branch.
 *
 * This module injects the reference motion — the same oscillator model the
 * authoring tool uses for baked wind — into any three.js material via
 * onBeforeCompile, so live shader wind matches `canopy-cli export --time ...`
 * frame for frame.
 *
 * Usage:
 *   import { applyCanopyWind, canopyWindUniforms } from './canopy-wind.js';
 *   const wind = canopyWindUniforms({ strength: 0.6, directionDeg: 25, gust: 0.5 });
 *   gltf.scene.traverse((o) => { if (o.isMesh) applyCanopyWind(o.material, wind); });
 *   // per frame: wind.uTime.value = clock.getElapsedTime();
 */

export function canopyWindUniforms({ strength = 0.5, directionDeg = 0, gust = 0.5 } = {}) {
  const radians = (directionDeg * Math.PI) / 180;
  return {
    uTime: { value: 0 },
    uWindStrength: { value: strength },
    uWindDir: { value: [Math.sin(radians), Math.cos(radians)] }, // xz plane
    uGust: { value: gust },
  };
}

const WIND_VERTEX_PARS = /* glsl */ `
attribute vec3 _wind_anchor;
attribute vec4 _wind_params;
uniform float uTime;
uniform float uWindStrength;
uniform vec2 uWindDir;
uniform float uGust;

vec3 canopyRotate(vec3 v, vec3 axis, float angle) {
  float c = cos(angle);
  float s = sin(angle);
  return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0 - c);
}

vec3 canopyWind(vec3 position) {
  float amplitude = _wind_params.x * uWindStrength;
  if (amplitude <= 0.0 && _wind_params.w < 1.5) return position; // anchored
  float phase = _wind_params.y;
  // Quadratic stiffness falloff: zero at the branch base, tip angle at the
  // end — the trunk base and ground-clamped roots never move.
  float along = clamp(distance(position, _wind_anchor) / max(_wind_params.z, 1e-3),
                      0.0, 1.0);
  float falloff = along * along;
  vec3 windDir = vec3(uWindDir.x, 0.0, uWindDir.y);
  vec3 bendAxis = normalize(cross(vec3(0.0, 1.0, 0.0), windDir));
  float osc = 0.7 * sin(1.7 * uTime + phase) + 0.3 * sin(3.9 * uTime + 2.7 * phase);
  float gustWave = sin(0.53 * uTime + 0.5 * phase);
  float gustEnvelope = 1.0 + uGust * max(0.0, gustWave) * gustWave * gustWave;
  float mainAngle = amplitude * gustEnvelope * (0.9 + 0.55 * osc) * falloff;
  float lateralAngle = amplitude * 0.4 * sin(1.3 * uTime + 1.9 * phase) * falloff;
  vec3 swayed = _wind_anchor +
      canopyRotate(canopyRotate(position - _wind_anchor, windDir, lateralAngle),
                   bendAxis, mainAngle);
  // Foliage ripple (kind 2): high-frequency shimmer on top of the sway the
  // parent branch already applies through its own vertices' channels.
  if (_wind_params.w > 1.5) {
    float vertexPhase = position.x * 5.1 + position.y * 3.7 + position.z * 4.3;
    swayed.y += 0.010 * uWindStrength * sin(6.1 * uTime + vertexPhase);
  }
  return swayed;
}
`;

export function applyCanopyWind(material, uniforms) {
  material.onBeforeCompile = (shader) => {
    Object.assign(shader.uniforms, uniforms);
    shader.vertexShader = WIND_VERTEX_PARS + shader.vertexShader;
    shader.vertexShader = shader.vertexShader.replace(
      '#include <begin_vertex>',
      '#include <begin_vertex>\n  transformed = canopyWind(transformed);'
    );
  };
  material.customProgramCacheKey = () => 'canopy-wind';
  material.needsUpdate = true;
  return material;
}

#!/usr/bin/env python3
"""Billboard impostor baker (ADR-0009, diagnostic grade until 14_LOD lands).

For each species: renders an RGBA sprite of the draft LOD with the diagnostic
rasterizer, then builds a two-quad crossed-billboard GLB embedding it.
Outputs samples/impostors/<name>.{png,glb}.
"""
import json
import os
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CLI = os.path.join(REPO, 'build', 'headless-dev', 'apps', 'cli', 'canopy-cli')
OUT = os.path.join(REPO, 'samples', 'impostors')

SPECIES = ['CoastalOak', 'AlpineFir', 'WeepingWillow', 'IslandPalm', 'RiverBirch']


def build_impostor_glb(png_path, glb_path, width, height):
    """Two crossed quads (X and Z planes), alpha-masked sprite, unlit-ish."""
    png = open(png_path, 'rb').read()
    hw = width / 2
    # Interleave positions (xyz), normals, uvs for 8 vertices (two quads).
    quads = [
        [(-hw, 0, 0), (hw, 0, 0), (hw, height, 0), (-hw, height, 0)],  # facing Z
        [(0, 0, -hw), (0, 0, hw), (0, height, hw), (0, height, -hw)],  # facing X
    ]
    normals = [(0, 0, 1), (1, 0, 0)]
    uv = [(0, 1), (1, 1), (1, 0), (0, 0)]  # glTF v runs top-down
    positions, norms, uvs, indices = [], [], [], []
    for q, quad in enumerate(quads):
        base = len(positions)
        positions += quad
        norms += [normals[q]] * 4
        uvs += uv
        indices += [base, base + 1, base + 2, base, base + 2, base + 3]

    def floats(values):
        return b''.join(struct.pack('<f', v) for pt in values for v in pt)

    pos_bytes = floats(positions)
    norm_bytes = floats(norms)
    uv_bytes = floats(uvs)
    idx_bytes = b''.join(struct.pack('<I', i) for i in indices)
    png_pad = (-len(png)) % 4
    bin_chunk = pos_bytes + norm_bytes + uv_bytes + idx_bytes + png + b'\0' * png_pad

    views = []
    offset = 0
    for blob in (pos_bytes, norm_bytes, uv_bytes, idx_bytes, png):
        views.append({'buffer': 0, 'byteOffset': offset, 'byteLength': len(blob)})
        offset += len(blob) + (png_pad if blob is png else 0)

    xs = [p[0] for p in positions]
    ys = [p[1] for p in positions]
    zs = [p[2] for p in positions]
    doc = {
        'asset': {'version': '2.0', 'generator': 'canopy make_impostors'},
        'buffers': [{'byteLength': len(bin_chunk)}],
        'bufferViews': views,
        'accessors': [
            {'bufferView': 0, 'componentType': 5126, 'count': 8, 'type': 'VEC3',
             'min': [min(xs), min(ys), min(zs)], 'max': [max(xs), max(ys), max(zs)]},
            {'bufferView': 1, 'componentType': 5126, 'count': 8, 'type': 'VEC3'},
            {'bufferView': 2, 'componentType': 5126, 'count': 8, 'type': 'VEC2'},
            {'bufferView': 3, 'componentType': 5125, 'count': len(indices),
             'type': 'SCALAR'},
        ],
        'images': [{'bufferView': 4, 'mimeType': 'image/png'}],
        'samplers': [{'magFilter': 9729, 'minFilter': 9729,
                      'wrapS': 33071, 'wrapT': 33071}],
        'textures': [{'sampler': 0, 'source': 0}],
        'materials': [{
            'name': 'impostor',
            'pbrMetallicRoughness': {
                'baseColorTexture': {'index': 0},
                'metallicFactor': 0, 'roughnessFactor': 1,
            },
            'alphaMode': 'MASK', 'alphaCutoff': 0.4, 'doubleSided': True,
        }],
        'meshes': [{'primitives': [{
            'attributes': {'POSITION': 0, 'NORMAL': 1, 'TEXCOORD_0': 2},
            'indices': 3, 'material': 0,
        }]}],
        'nodes': [{'mesh': 0}],
        'scenes': [{'nodes': [0]}],
        'scene': 0,
    }
    json_chunk = json.dumps(doc, separators=(',', ':')).encode()
    json_chunk += b' ' * ((-len(json_chunk)) % 4)
    total = 12 + 8 + len(json_chunk) + 8 + len(bin_chunk)
    with open(glb_path, 'wb') as f:
        f.write(b'glTF' + struct.pack('<II', 2, total))
        f.write(struct.pack('<I', len(json_chunk)) + b'JSON' + json_chunk)
        f.write(struct.pack('<I', len(bin_chunk)) + b'BIN\0' + bin_chunk)


def main():
    os.makedirs(OUT, exist_ok=True)
    for name in SPECIES:
        base = os.path.join(REPO, 'build', 'impostor-src', name)
        preset = base + '.preset.json'
        os.makedirs(os.path.dirname(base), exist_ok=True)
        with open(preset, 'w') as f:
            f.write('{"format": "obj", "profile": "draft"}\n')
        subprocess.run([CLI, 'export',
                        os.path.join(REPO, 'samples', 'documents', f'{name}.canopyproj'),
                        '--preset', preset, '--out', base],
                       check=True, capture_output=True)
        sprite = os.path.join(OUT, f'{name}.png')
        render([load_obj(base + '.obj')], sprite, w=384, h=512, azim_deg=0, fov=18,
               transparent=True)
        verts = load_obj(base + '.obj')[0]
        height = float(verts[:, 1].max())
        width = float(max(verts[:, 0].max() - verts[:, 0].min(),
                          verts[:, 2].max() - verts[:, 2].min()))
        build_impostor_glb(sprite, os.path.join(OUT, f'{name}.glb'), width, height)
        print(f'{name}: impostor {width:.1f}x{height:.1f} m')


if __name__ == '__main__':
    main()

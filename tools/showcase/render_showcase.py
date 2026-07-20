#!/usr/bin/env python3
"""Flat-shaded diagnostic renderer for the showcase OBJ exports.

Writes the reference images in samples/renders/ from build/showcase/*.obj.
Requires numpy and Pillow. See tools/showcase/generate_documents.py for the
full regeneration pipeline. The WebGPU viewport replaces this eventually
(ADR-0005).
"""
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
import sys, os

def load_mtl(path):
    """name -> {'color': (r,g,b), 'map': texture path or None}"""
    materials, current = {}, None
    base_dir = os.path.dirname(path)
    try:
        for line in open(path):
            p = line.split()
            if not p:
                continue
            if p[0] == 'newmtl':
                current = p[1]
                materials[current] = {'color': (150, 120, 90), 'map': None}
            elif p[0] == 'Kd' and current:
                materials[current]['color'] = tuple(int(float(x) * 255) for x in p[1:4])
            elif p[0] == 'map_Kd' and current:
                materials[current]['map'] = os.path.join(base_dir, p[1])
    except FileNotFoundError:
        pass
    return materials

def load_obj(path, offset=(0, 0, 0)):
    verts, uvs, faces, mats, face_uv = [], [], [], [], []
    current = 'default'
    mtl_info = load_mtl(path.replace('.obj', '.mtl'))
    for line in open(path):
        p = line.split()
        if not p:
            continue
        if p[0] == 'v':
            verts.append([float(p[1]) + offset[0], float(p[2]) + offset[1],
                          float(p[3]) + offset[2]])
        elif p[0] == 'vt':
            uvs.append([float(p[1]), float(p[2])])
        elif p[0] == 'usemtl':
            current = p[1]
        elif p[0] == 'f':
            corners = [t.split('/') for t in p[1:4]]
            faces.append([int(c[0]) - 1 for c in corners])
            mats.append(current)
            if len(corners[0]) > 1 and corners[0][1]:
                us = [uvs[int(c[1]) - 1] for c in corners]
                face_uv.append([(us[0][0] + us[1][0] + us[2][0]) / 3,
                                (us[0][1] + us[1][1] + us[2][1]) / 3])
            else:
                face_uv.append(None)
    return np.array(verts), np.array(faces), mats, mtl_info, face_uv

def render(objs, out_png, w=1000, h=1250, azim_deg=35, fov=32, zoom=1.0, bases=None,
           focus_center=None, focus_size=None,
           bark=(112, 87, 62), palette=None, transparent=False,
           sky_top=(178, 205, 228), sky_bot=(238, 240, 236), ground=(206, 208, 198)):
    V = np.vstack([o[0] for o in objs])
    F = np.vstack([o[1] + sum(len(p[0]) for p in objs[:i]) for i, o in enumerate(objs)])
    M = [m for o in objs for m in o[2]]
    FUV = [uv for o in objs for uv in o[4]]
    mtl_info = {}
    for o in objs:
        mtl_info.update(o[3])
    # Per-face colors: texture sample at the UV centroid when the material
    # has a map (alpha-masked faces below 0.4 are dropped), flat Kd otherwise.
    texture_cache = {}
    def texture_pixels(path):
        if path not in texture_cache:
            img = Image.open(path).convert('RGBA')
            texture_cache[path] = np.asarray(img, dtype=np.uint8)
        return texture_cache[path]

    lo, hi = V.min(0), V.max(0)
    center = np.array(focus_center, float) if focus_center is not None else (lo + hi) / 2
    size = float(focus_size) if focus_size is not None else float(np.max(hi - lo))
    az = np.radians(azim_deg)
    elev = np.radians(8)
    dist = size * 1.45 / np.tan(np.radians(fov / 2)) * 0.62 / zoom
    eye = center + dist * np.array([np.sin(az) * np.cos(elev), np.sin(elev),
                                    np.cos(az) * np.cos(elev)])
    fwd = center - eye
    fwd /= np.linalg.norm(fwd)
    right = np.cross(fwd, [0, 1, 0]); right /= np.linalg.norm(right)
    up = np.cross(right, fwd)

    C = V - eye
    cam = np.stack([C @ right, C @ up, C @ fwd], 1)
    f = 1.0 / np.tan(np.radians(fov / 2))
    z = np.maximum(cam[:, 2], 1e-6)
    sx = (cam[:, 0] * f / z * (h / w)) * (w / 2) + w / 2
    sy = h / 2 - cam[:, 1] * f / z * (h / 2)

    tri = cam[F]                      # (n,3,3) camera-space triangles
    depth = tri[:, :, 2].mean(1)
    n = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    nl = np.linalg.norm(n, axis=1, keepdims=True)
    n = n / np.maximum(nl, 1e-12)
    light = np.array([-0.45, 0.8, -0.4]); light /= np.linalg.norm(light)
    # transform light into camera space
    lcam = np.array([light @ right, light @ up, light @ fwd])
    lam = np.clip(n @ lcam, 0, None)
    # height-based ambient tint (fake sky bounce)
    hgt = (V[F][:, :, 1].mean(1) - lo[1]) / max(size, 1e-9)
    shade = np.clip(0.32 + 0.62 * np.abs(lam) + 0.10 * hgt, 0, 1.15)

    w, h = w * 2, h * 2
    sx, sy = sx * 2, sy * 2
    img = Image.new('RGB', (w, h))
    if transparent:
        img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        overrides = palette or {}
        # geometry only — no sky, no ground
        order2 = np.argsort(-depth)
        P2 = np.stack([sx, sy], 1)
        M2 = [m for o in objs for m in o[2]]
        FUV2 = [uv for o in objs for uv in o[4]]
        mtl_info2 = {}
        for o in objs:
            mtl_info2.update(o[3])
        cache2 = {}
        for i in order2:
            a, b, c = F[i]
            s = shade[i]
            info = mtl_info2.get(M2[i])
            base_col = overrides.get(M2[i]) or (info['color'] if info else bark)
            if info and info.get('map') and FUV2[i] is not None:
                if info['map'] not in cache2:
                    cache2[info['map']] = np.asarray(
                        Image.open(info['map']).convert('RGBA'), dtype=np.uint8)
                pixels = cache2[info['map']]
                th, tw = pixels.shape[0], pixels.shape[1]
                u = FUV2[i][0] % 1.0
                v = FUV2[i][1] % 1.0
                px = pixels[min(th - 1, int((1.0 - v) * th)), min(tw - 1, int(u * tw))]
                if px[3] < 100:
                    continue
            col = tuple(int(min(255, base_col[k] * s)) for k in range(3)) + (255,)
            draw.polygon([tuple(P2[a]), tuple(P2[b]), tuple(P2[c])], fill=col)
        img = img.resize((w // 2, h // 2), Image.LANCZOS)
        img.save(out_png)
        print(f'rendered {out_png} ({len(F)} tris, transparent)')
        return
    # vertical sky gradient
    grad = np.linspace(0, 1, h)[:, None] * np.ones((1, w))
    arr = np.zeros((h, w, 3), np.uint8)
    for c in range(3):
        arr[:, :, c] = (sky_top[c] * (1 - grad) + sky_bot[c] * grad).astype(np.uint8)
    img = Image.fromarray(arr)
    draw = ImageDraw.Draw(img)

    # soft ground ellipse
    if bases is None:
        bases = [(0.0, 0.0, 0.0)]
    B = np.array(bases, float) - eye
    bcam = np.stack([B @ right, B @ up, B @ fwd], 1)
    bz = np.maximum(bcam[:, 2], 1e-6)
    bsy = (h / 2 - bcam[:, 1] * f / bz * (h / 2))
    top = float(bsy.min()) - h * 0.02
    bot = float(bsy.max()) + h * 0.05
    draw.ellipse([w * 0.10, top, w * 0.90, bot], fill=ground)

    order = np.argsort(-depth)  # far to near
    P = np.stack([sx, sy], 1)
    overrides = palette or {}
    for i in order:
        a, b, c = F[i]
        s = shade[i]
        info = mtl_info.get(M[i])
        base_col = overrides.get(M[i]) or (info['color'] if info else bark)
        if info and info.get('map') and FUV[i] is not None:
            pixels = texture_pixels(info['map'])
            th, tw = pixels.shape[0], pixels.shape[1]
            u = FUV[i][0] % 1.0
            v = FUV[i][1] % 1.0
            px = pixels[min(th - 1, int((1.0 - v) * th)), min(tw - 1, int(u * tw))]
            if px[3] < 100:
                continue  # alpha-masked out
            base_col = (int(px[0]), int(px[1]), int(px[2]))
        col = tuple(int(min(255, base_col[k] * s)) for k in range(3))
        draw.polygon([tuple(P[a]), tuple(P[b]), tuple(P[c])], fill=col)

    img = img.resize((w // 2, h // 2), Image.LANCZOS)
    img.save(out_png, quality=90)
    print(f"rendered {out_png} ({len(F)} tris)")

if __name__ == '__main__':
    repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    base = os.path.join(repo, 'build', 'showcase')
    out = os.path.join(repo, 'samples', 'renders')
    os.makedirs(out, exist_ok=True)
    render([load_obj(f'{base}/CoastalOak.obj')], f'{out}/coastal_oak.png', azim_deg=30)
    render([load_obj(f'{base}/CoastalOak.obj')], f'{out}/oak_base.png', w=1400, h=950,
           azim_deg=40, fov=30, focus_center=(0, 0.62, 0), focus_size=2.6,
           bases=[(0,0,0),(1.9,0,1.9),(-1.9,0,-1.9),(1.9,0,-1.9),(-1.9,0,1.9)])
    render([load_obj(f'{base}/AlpineFir.obj')], f'{out}/alpine_fir.png', azim_deg=55)
    render([load_obj(f'{base}/WeepingWillow.obj')], f'{out}/weeping_willow.png', azim_deg=20)
    render([load_obj(f'{base}/IslandPalm.obj')], f'{out}/island_palm.png', azim_deg=25)
    render([load_obj(f'{base}/RiverBirch.obj')], f'{out}/river_birch.png', azim_deg=32)
    render([load_obj(f'{base}/NebulaSporeTree.obj')], f'{out}/nebula_spore_tree.png',
           azim_deg=28, sky_top=(46, 34, 78), sky_bot=(196, 116, 132),
           ground=(96, 74, 96))
    render([load_obj(f'{base}/CoastalOak.obj', (-10, 0, 0.5)),
            load_obj(f'{base}/AlpineFir.obj', (-3.5, 0, -3.5)),
            load_obj(f'{base}/RiverBirch.obj', (2, 0, 1.5)),
            load_obj(f'{base}/WeepingWillow.obj', (7.5, 0, -1)),
            load_obj(f'{base}/IslandPalm.obj', (13.5, 0, -2.5))],
           f'{out}/grove.png', w=1700, h=1000, azim_deg=8, fov=38, zoom=1.5,
           bases=[(-10, 0, 0.5), (-3.5, 0, -3.5), (2, 0, 1.5), (7.5, 0, -1), (13.5, 0, -2.5)])

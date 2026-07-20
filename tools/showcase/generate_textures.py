#!/usr/bin/env python3
"""Procedural texture assets for the showcase projects (ADR-0009).

Deterministic (fixed seed), dependency-light (numpy + Pillow), provenance
clean: everything is synthesized here, nothing is sourced externally.
Generates per project:

  assets/textures/bark.png        512x512 tileable-ish bark albedo
  assets/textures/bark_normal.png tangent-space normal map from the ridges
  assets/textures/leaf_atlas.png  1024x1024 RGBA atlas, 2x2 regions:
      [0,0]-[.5,.5]  oak leaf     [.5,0]-[1,.5]  birch leaf
      [0,.5]-[.5,1]  willow leaf  [.5,.5]-[1,1]  fir needle spray
"""
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


def _value_noise(shape, cells, rng):
    coarse = rng.random((cells, cells))
    img = Image.fromarray((coarse * 255).astype(np.uint8), 'L')
    img = img.resize(shape, Image.BICUBIC)
    return np.asarray(img, dtype=np.float64) / 255.0


def bark_height(size=512, seed=7):
    rng = np.random.default_rng(seed)
    height = np.zeros((size, size))
    # Vertical ridges: stretch noise strongly along Y, layered octaves.
    for cells, weight in ((4, 0.5), (8, 0.3), (24, 0.2)):
        noise = _value_noise((size // 4, size), cells, rng)
        noise = np.asarray(
            Image.fromarray((noise * 255).astype(np.uint8), 'L').resize((size, size),
                                                                        Image.BICUBIC),
            dtype=np.float64) / 255.0
        height += weight * noise
    # Horizontal cracks.
    cracks = _value_noise((size, size // 8), 12, rng)
    cracks = np.asarray(
        Image.fromarray((cracks * 255).astype(np.uint8), 'L').resize((size, size),
                                                                     Image.BICUBIC),
        dtype=np.float64) / 255.0
    height = 0.85 * height + 0.15 * cracks
    return (height - height.min()) / (height.max() - height.min() + 1e-9)


def write_bark(directory, seed=7):
    size = 512
    height = bark_height(size, seed)
    shade = 0.55 + 0.45 * height
    base = np.array([0.42, 0.33, 0.24])
    albedo = (shade[..., None] * base[None, None, :] * 255).astype(np.uint8)
    Image.fromarray(albedo, 'RGB').save(os.path.join(directory, 'bark.png'), optimize=True)

    # Tangent-space normal from the height gradient.
    gy, gx = np.gradient(height * 24.0)
    nz = np.ones_like(height)
    length = np.sqrt(gx * gx + gy * gy + nz * nz)
    normal = np.stack([(-gx / length * 0.5 + 0.5), (gy / length * 0.5 + 0.5),
                       (nz / length * 0.5 + 0.5)], axis=-1)
    Image.fromarray((normal * 255).astype(np.uint8), 'RGB').save(
        os.path.join(directory, 'bark_normal.png'), optimize=True)


def _draw_leaf(draw, outline, box, fill, vein):
    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0
    # Outline is stem→tip in (x −0.5..0.5, y 0..1); atlas y is image-down, so
    # the stem sits at the region's bottom.
    points = [(x0 + (0.5 + px) * w, y1 - py * h) for px, py in outline]
    draw.polygon(points, fill=fill)
    # Midrib + side veins.
    stem = (x0 + 0.5 * w, y1)
    tip = (x0 + 0.5 * w, y0 + 0.04 * h)
    draw.line([stem, tip], fill=vein, width=max(2, w // 90))
    for frac in (0.25, 0.45, 0.65, 0.8):
        base = (x0 + 0.5 * w, y1 - frac * h)
        spread = 0.30 * w * (1.0 - frac * 0.7)
        rise = 0.12 * h
        for side in (-1, 1):
            draw.line([base, (base[0] + side * spread, base[1] - rise)], fill=vein,
                      width=max(1, w // 160))


def write_leaf_atlas(directory, outlines):
    size = 1024
    half = size // 2
    image = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    pad = 40
    _draw_leaf(draw, outlines['oak'], (pad, pad, half - pad, half - pad),
               (74, 110, 44, 255), (52, 82, 30, 255))
    _draw_leaf(draw, outlines['birch'], (half + pad, pad, size - pad, half - pad),
               (104, 138, 56, 255), (76, 104, 40, 255))
    _draw_leaf(draw, outlines['willow'], (pad, half + pad, half - pad, size - pad),
               (124, 146, 70, 255), (94, 114, 52, 255))
    # Fir needle spray: a twig with needle strokes.
    x0, y0, x1, y1 = (half + pad, half + pad, size - pad, size - pad)
    cx = (x0 + x1) // 2
    draw.line([(cx, y1), (cx, y0 + 30)], fill=(96, 74, 52, 255), width=8)
    rng = np.random.default_rng(11)
    for i in range(46):
        t = i / 45.0
        y = y1 - t * (y1 - y0 - 40)
        reach = (0.42 - 0.3 * t) * (x1 - x0)
        for side in (-1, 1):
            jitter = float(rng.uniform(-8, 8))
            draw.line([(cx, y), (cx + side * reach, y - 34 + jitter)],
                      fill=(44, 84, 56, 255), width=6)
    image = image.filter(ImageFilter.SMOOTH)
    image.save(os.path.join(directory, 'leaf_atlas.png'), optimize=True)


def ensure_textures(project_dir, outlines):
    directory = os.path.join(project_dir, 'assets', 'textures')
    os.makedirs(directory, exist_ok=True)
    write_bark(directory)
    write_leaf_atlas(directory, outlines)

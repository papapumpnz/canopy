#!/usr/bin/env python3
"""Render the cartoon tree concepts exported by canopy-cli.

Writes into samples/cartoon/renders/:

  <species>.png            hero still on a storybook backdrop
  <species>_turntable.gif  24-frame Y-axis turntable loop
  cartoon_large.png        contact sheet, the three large canopied trees
  cartoon_small.png        contact sheet, the medium and small trees
  cartoon_lineup.png       all seven to a common scale, for size comparison

Frames come from the diagnostic CPU rasterizer in render_showcase.py, which
is a stand-in until the WebGPU viewport lands (ADR-0005).  Needs numpy and
Pillow.
"""

from __future__ import annotations

import math
import os
import sys
from concurrent.futures import ProcessPoolExecutor

import numpy as np
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE = os.path.join(REPO, "build", "cartoon")
OUTPUT = os.path.join(REPO, "samples", "cartoon", "renders")
FRAMES = os.path.join(REPO, "build", "cartoon", "frames")

# name, slug, hero azimuth, class, label
CONCEPTS = (
    ("ToonBigTop", "big_top", 34, "large", "Large · Big Top"),
    ("ToonBroadSpread", "broad_spread", 26, "large", "Large · Broad Spread"),
    ("ToonTallStack", "tall_stack", 30, "large", "Large · Tall Stack"),
    ("ToonRoundTop", "round_top", 30, "small", "Medium · Round Top"),
    # Shot in profile: the lean is this species' whole silhouette, and it
    # collapses into a shapeless blob viewed down the axis of the lean.
    ("ToonLeaner", "leaner", 200, "small", "Medium · Leaner"),
    ("ToonSapling", "sapling", 28, "small", "Small · Sapling"),
    ("ToonBush", "bush", 24, "small", "Small · Bush"),
)

TURNTABLE_FRAMES = 18
TURNTABLE_MS = 110
GIF_SIZE = (400, 420)
HERO_SIZE = (700, 780)

# Both outputs are palette-indexed. The art is flat-shaded with a small ramp,
# so these counts are visually lossless — measured against the full-colour
# renders, 64 is the floor before the trunk browns start drifting olive.
GIF_COLOURS = 64
PNG_COLOURS = 256

SKY_TOP = (128, 196, 240)
SKY_BOTTOM = (226, 244, 236)
GRASS = (124, 190, 100)
GRASS_RIM = (92, 158, 74)


def frame_path(slug: str, index: int) -> str:
    return os.path.join(FRAMES, f"{slug}_{index:03d}.png")


def turntable_framing(vertices, width: int, height: int) -> tuple[tuple[float, float, float], float]:
    """Camera target and focus size for a turntable that neither drifts nor
    breathes.

    The target sits on the trunk's rotation axis (x = z = 0), NOT at the
    bounding-box centre: an asymmetric crown would otherwise drag the whole
    image sideways as the model spins.  The focus size is solved so the
    rotation envelope — the widest the silhouette ever gets, which is twice
    the largest horizontal radius — fits the frame at every azimuth.
    """
    low, high = vertices.min(0), vertices.max(0)
    span = float(high[1] - low[1])
    radius = float(((vertices[:, 0] ** 2 + vertices[:, 2] ** 2) ** 0.5).max())
    # render() shows a frustum 1.798*focus tall and (w/h) times that wide.
    focus = max(span * 1.08, (2.0 * radius * 1.06) / (width / height)) / 1.798
    return (0.0, float(low[1]) + span / 2.0, 0.0), focus


def render_frame(job: tuple[str, str, int, float, int, int, str]) -> str:
    """Worker entry point: one alpha-only frame of one species."""
    name, slug, index, azimuth, width, height, mode = job
    obj = load_obj(os.path.join(SOURCE, f"{name}.obj"))
    framing = turntable_framing if mode == "turntable" else still_framing
    centre, focus = framing(obj[0], width, height)
    out = frame_path(slug, index)
    render(
        [obj],
        out,
        w=width,
        h=height,
        azim_deg=azimuth,
        fov=32,
        zoom=1.0,
        focus_center=centre,
        focus_size=focus,
        transparent=True,
    )
    return out


def storybook_backdrop(size: tuple[int, int], ground: tuple[int, int, int, int]) -> Image.Image:
    """Flat cartoon sky with a grass disc, built once and reused by every
    frame so the ground does not shimmer under the rotating tree."""
    width, height = size
    backdrop = Image.new("RGB", size, SKY_BOTTOM)
    draw = ImageDraw.Draw(backdrop)
    for y in range(height):
        blend = y / max(height - 1, 1)
        # Two flat bands with a short ramp between them reads more like
        # painted sky than a full-height gradient.
        ramp = min(1.0, max(0.0, (blend - 0.15) / 0.5))
        draw.line(
            [(0, y), (width, y)],
            fill=tuple(
                int(SKY_TOP[c] * (1.0 - ramp) + SKY_BOTTOM[c] * ramp) for c in range(3)
            ),
        )
    left, top, right, bottom = ground
    draw.ellipse((left, top, right, bottom), fill=GRASS_RIM)
    draw.ellipse((left + 4, top + 3, right - 4, bottom - 3), fill=GRASS)
    return backdrop


def project(
    point: tuple[float, float, float],
    centre: tuple[float, float, float],
    focus: float,
    width: int,
    height: int,
    azimuth: float,
    fov: float = 32.0,
    zoom: float = 1.0,
) -> tuple[float, float]:
    """Project a world point to screen space, mirroring render()'s camera.

    Used to place the grass on the trunk foot.  Deriving the ground line from
    the silhouette instead makes it chase whichever azimuth hangs the crown
    lowest, so a tree with an off-centre crown floats for most of the loop.
    """
    elevation = math.radians(8.0)
    half_fov = math.radians(fov / 2.0)
    distance = focus * 1.45 / math.tan(half_fov) * 0.62 / zoom
    azimuth = math.radians(azimuth)
    centre = np.asarray(centre, dtype=float)
    eye = centre + distance * np.array(
        [
            math.sin(azimuth) * math.cos(elevation),
            math.sin(elevation),
            math.cos(azimuth) * math.cos(elevation),
        ]
    )
    forward = centre - eye
    forward /= np.linalg.norm(forward)
    right = np.cross(forward, [0.0, 1.0, 0.0])
    right /= np.linalg.norm(right)
    up = np.cross(right, forward)
    offset = np.asarray(point, dtype=float) - eye
    camera = np.array([offset @ right, offset @ up, offset @ forward])
    scale = 1.0 / math.tan(half_fov)
    depth = max(camera[2], 1e-6)
    return (
        camera[0] * scale / depth * (height / width) * (width / 2) + width / 2,
        height / 2 - camera[1] * scale / depth * (height / 2),
    )


def still_framing(vertices, width: int, height: int) -> tuple[tuple[float, float, float], float]:
    """Camera target and focus size for a one-off hero shot.

    Unlike the turntable this centres on the bounding box, so a lopsided tree
    is composed in the frame rather than shoved to one side by its trunk axis.
    """
    low, high = vertices.min(0), vertices.max(0)
    centre = tuple((low + high) / 2)
    span = float(high[1] - low[1])
    spread = float(max(high[0] - low[0], high[2] - low[2]))
    focus = max(span * 1.16, spread * 1.12 / (width / height)) / 1.798
    return centre, focus


def ground_disc(
    frames: list[Image.Image],
    thickness: int | None = None,
    base: float | None = None,
) -> tuple[int, int, int, int]:
    """A grass disc spanning the union of every frame's silhouette, resting on
    `base` (defaulting to the lowest pixel any frame reaches)."""
    width, height = frames[0].size
    left, right, silhouette_base = width, 0, 0
    for frame in frames:
        bounds = frame.getchannel("A").getbbox()
        if bounds is None:
            continue
        left = min(left, bounds[0])
        right = max(right, bounds[2])
        silhouette_base = max(silhouette_base, bounds[3])
    if base is None:
        base = silhouette_base
    pad = max(20, int((right - left) * 0.1))
    if thickness is None:
        thickness = max(14, height // 40)
    return (
        max(0, int(left - pad)),
        int(base - thickness),
        min(width, int(right + pad)),
        min(height, int(base + thickness)),
    )


def compose(
    frames: list[Image.Image], disc: tuple[int, int, int, int] | None = None
) -> list[Image.Image]:
    backdrop = storybook_backdrop(frames[0].size, disc or ground_disc(frames))
    composed = []
    for frame in frames:
        panel = backdrop.copy().convert("RGBA")
        panel.alpha_composite(frame)
        composed.append(panel.convert("RGB"))
    return composed


def save_png(image: Image.Image, path: str) -> None:
    """Write a palette-indexed PNG. Roughly a third the size of truecolour
    on this flat-shaded art, with no visible difference."""
    image.convert("RGB").quantize(
        colors=PNG_COLOURS, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE
    ).save(path, optimize=True)
    print(f"wrote {path} ({os.path.getsize(path) // 1024} KB)")


def save_gif(frames: list[Image.Image], path: str, duration_ms: int) -> None:
    """Write the loop under one shared adaptive palette.

    Dithering is off: it would scatter pixels that are otherwise identical
    between frames and defeat Pillow's inter-frame bounding-box optimisation.
    Encoding the static backdrop as transparent pixels was tried and does not
    pay — it breaks up the LZW runs by more than the redundancy it removes.
    """
    palette = frames[0].quantize(colors=GIF_COLOURS, method=Image.Quantize.MEDIANCUT)
    quantized = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in frames]
    quantized[0].save(
        path,
        save_all=True,
        append_images=quantized[1:],
        duration=duration_ms,
        loop=0,
        optimize=True,
    )
    print(f"wrote {path} ({os.path.getsize(path) // 1024} KB, {len(frames)} frames)")


def contact_sheet(title: str, entries: tuple[tuple[str, str], ...], output_name: str) -> None:
    tile_width, tile_height = 480, 580
    header = 82
    sheet = Image.new("RGB", (tile_width * len(entries), header + tile_height), (26, 42, 30))
    draw = ImageDraw.Draw(sheet)
    draw.text((26, 26), title, fill=(226, 240, 214), font=ImageFont.load_default(size=24))
    label_font = ImageFont.load_default(size=17)
    for index, (slug, label) in enumerate(entries):
        image = Image.open(os.path.join(OUTPUT, f"{slug}.png")).convert("RGB")
        image.thumbnail((tile_width - 22, tile_height - 46), Image.Resampling.LANCZOS)
        sheet.paste(image, (index * tile_width + (tile_width - image.width) // 2, header + 4))
        draw.text(
            (index * tile_width + 16, header + tile_height - 32),
            label,
            fill=(226, 240, 214),
            font=label_font,
        )
    save_png(sheet, os.path.join(OUTPUT, output_name))


def lineup() -> None:
    """Every species in one shot at a shared scale, so the large/medium/small
    relationship is legible rather than implied by separate framings.

    This goes through the same alpha path as the stills.  The opaque branch
    of the rasterizer samples one texel at each triangle's UV centroid, which
    turns every foliage card into a solid triangle and shreds the silhouette.
    """
    width, height = 1900, 560
    objects, cursor = [], 0.0
    for name, *_ in CONCEPTS:
        probe = load_obj(os.path.join(SOURCE, f"{name}.obj"))
        low, high = probe[0].min(0), probe[0].max(0)
        half = float(max(high[0] - low[0], high[2] - low[2])) / 2.0
        cursor += half + 0.45
        objects.append(load_obj(os.path.join(SOURCE, f"{name}.obj"), (cursor, 0.0, 0.0)))
        cursor += half

    vertices = [obj[0] for obj in objects]
    low = tuple(min(v.min(0)[i] for v in vertices) for i in range(3))
    high = tuple(max(v.max(0)[i] for v in vertices) for i in range(3))
    row_width = high[0] - low[0]
    row_height = high[1] - low[1]
    centre = tuple((low[i] + high[i]) / 2 for i in range(3))
    # render() derives the camera distance from focus_size, showing a frustum
    # 1.798*size tall and (w/h) times that wide at zoom 1. Solve for the size
    # that just contains the row in both axes instead of guessing a zoom.
    focus = max(row_height * 1.16 / 1.798, row_width * 1.18 * height / (1.798 * width))

    frame_file = os.path.join(FRAMES, "lineup.png")
    # Head-on. A non-zero azimuth orbits the camera around the row centre,
    # which pushes a row this long off-centre and clips the far end.
    render(
        objects,
        frame_file,
        w=width,
        h=height,
        azim_deg=0,
        fov=34,
        zoom=1.0,
        focus_center=centre,
        focus_size=focus,
        transparent=True,
    )
    frame = Image.open(frame_file).convert("RGBA")
    save_png(
        compose([frame], ground_disc([frame], thickness=height // 11))[0],
        os.path.join(OUTPUT, "cartoon_lineup.png"),
    )


def main() -> None:
    os.makedirs(OUTPUT, exist_ok=True)
    os.makedirs(FRAMES, exist_ok=True)

    jobs = []
    for name, slug, hero_azimuth, _, _ in CONCEPTS:
        for index in range(TURNTABLE_FRAMES):
            azimuth = hero_azimuth + index * (360.0 / TURNTABLE_FRAMES)
            jobs.append((name, slug, index, azimuth, *GIF_SIZE, "turntable"))
        jobs.append((name, f"{slug}_hero", 0, float(hero_azimuth), *HERO_SIZE, "still"))

    workers = max(1, min(8, (os.cpu_count() or 2) - 1))
    with ProcessPoolExecutor(max_workers=workers) as pool:
        list(pool.map(render_frame, jobs, chunksize=4))

    for name, slug, hero_azimuth, _, _ in CONCEPTS:
        vertices = load_obj(os.path.join(SOURCE, f"{name}.obj"))[0]

        turntable = [
            Image.open(frame_path(slug, index)).convert("RGBA")
            for index in range(TURNTABLE_FRAMES)
        ]
        centre, focus = turntable_framing(vertices, *GIF_SIZE)
        base = project((0.0, 0.0, 0.0), centre, focus, *GIF_SIZE, 0.0)[1]
        save_gif(
            compose(turntable, ground_disc(turntable, base=base)),
            os.path.join(OUTPUT, f"{slug}_turntable.gif"),
            TURNTABLE_MS,
        )

        hero = Image.open(frame_path(f"{slug}_hero", 0)).convert("RGBA")
        centre, focus = still_framing(vertices, *HERO_SIZE)
        base = project((0.0, 0.0, 0.0), centre, focus, *HERO_SIZE, float(hero_azimuth))[1]
        save_png(
            compose([hero], ground_disc([hero], base=base))[0],
            os.path.join(OUTPUT, f"{slug}.png"),
        )

    contact_sheet(
        "CARTOON CANOPY · large trees",
        tuple((slug, label) for _, slug, _, group, label in CONCEPTS if group == "large"),
        "cartoon_large.png",
    )
    contact_sheet(
        "CARTOON CANOPY · medium and small",
        tuple((slug, label) for _, slug, _, group, label in CONCEPTS if group == "small"),
        "cartoon_small.png",
    )
    lineup()


if __name__ == "__main__":
    main()

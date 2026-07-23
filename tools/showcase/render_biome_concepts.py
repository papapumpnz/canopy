#!/usr/bin/env python3
"""Render the six standalone biome concepts exported by canopy-cli."""

from __future__ import annotations

import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render


REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE = os.path.join(REPO, "build", "biome-concepts")
OUTPUT = os.path.join(REPO, "samples", "concepts", "renders")

CONCEPTS = (
    ("VerdantCathedral", "verdant_cathedral.png", 28, "Major · Cathedral"),
    ("VerdantLantern", "verdant_lantern.png", 38, "Minor · Lantern"),
    ("VerdantThicket", "verdant_thicket.png", 24, "Shrub · Thicket"),
    ("AshEmberCrown", "ash_ember_crown.png", 31, "Major · Ember Crown"),
    ("AshDustwood", "ash_dustwood.png", 42, "Minor · Dustwood"),
    ("AshSpearbush", "ash_spearbush.png", 30, "Shrub · Spearbush"),
)


def scenic_backdrop(
    tree: Image.Image,
    *,
    sky_top: tuple[int, int, int],
    sky_bottom: tuple[int, int, int],
    ground: tuple[int, int, int],
) -> Image.Image:
    width, height = tree.size
    blend = np.linspace(0.0, 1.0, height, dtype=np.float64)[:, None, None]
    top = np.asarray(sky_top, dtype=np.float64)[None, None, :]
    bottom = np.asarray(sky_bottom, dtype=np.float64)[None, None, :]
    pixels = np.broadcast_to(top * (1.0 - blend) + bottom * blend, (height, width, 3))
    backdrop = Image.fromarray(pixels.astype(np.uint8), "RGB").convert("RGBA")

    alpha_bounds = tree.getchannel("A").getbbox()
    if alpha_bounds is not None:
        left, _, right, base_y = alpha_bounds
        pad = max(26, int((right - left) * 0.16))
        shadow = Image.new("RGBA", tree.size, (0, 0, 0, 0))
        draw = ImageDraw.Draw(shadow)
        draw.ellipse(
            (
                max(0, left - pad),
                base_y - max(16, height // 52),
                min(width, right + pad),
                min(height, base_y + max(17, height // 46)),
            ),
            fill=(*ground, 235),
        )
        backdrop.alpha_composite(shadow)
    backdrop.alpha_composite(tree)
    return backdrop.convert("RGB")


def contact_sheet(
    title: str,
    entries: tuple[tuple[str, str], ...],
    output_name: str,
    background: tuple[int, int, int],
    foreground: tuple[int, int, int],
) -> None:
    tile_width, tile_height = 520, 610
    header = 88
    sheet = Image.new("RGB", (tile_width * len(entries), header + tile_height), background)
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default(size=24)
    label_font = ImageFont.load_default(size=18)
    draw.text((28, 24), title, fill=foreground, font=font)
    for index, (filename, label) in enumerate(entries):
        image = Image.open(os.path.join(OUTPUT, filename)).convert("RGB")
        image.thumbnail((tile_width - 24, tile_height - 48), Image.Resampling.LANCZOS)
        x = index * tile_width + (tile_width - image.width) // 2
        y = header + 4
        sheet.paste(image, (x, y))
        draw.text(
            (index * tile_width + 18, header + tile_height - 35),
            label,
            fill=foreground,
            font=label_font,
        )
    sheet.save(os.path.join(OUTPUT, output_name), optimize=True)


def main() -> None:
    os.makedirs(OUTPUT, exist_ok=True)
    alpha_output = os.path.join(SOURCE, "alpha-renders")
    os.makedirs(alpha_output, exist_ok=True)
    for source_name, filename, azimuth, _ in CONCEPTS:
        ash = source_name.startswith("Ash")
        alpha_path = os.path.join(alpha_output, filename)
        render(
            [load_obj(os.path.join(SOURCE, f"{source_name}.obj"))],
            alpha_path,
            w=720,
            h=860,
            azim_deg=azimuth,
            fov=31 if "Thicket" not in source_name and "Spearbush" not in source_name else 34,
            zoom=1.12,
            transparent=True,
            foliage_alpha_erosion=3,
        )
        scenic_backdrop(
            Image.open(alpha_path).convert("RGBA"),
            sky_top=(193, 212, 218) if not ash else (215, 181, 146),
            sky_bottom=(237, 239, 224) if not ash else (236, 213, 174),
            ground=(160, 180, 142) if not ash else (166, 111, 75),
        ).save(os.path.join(OUTPUT, filename), optimize=True)

    contact_sheet(
        "VERDANT · canopy concepts",
        tuple((filename, label) for _, filename, _, label in CONCEPTS[:3]),
        "verdant_concepts.png",
        (31, 48, 37),
        (225, 235, 218),
    )
    contact_sheet(
        "ASH · dryland canopy concepts",
        tuple((filename, label) for _, filename, _, label in CONCEPTS[3:]),
        "ash_concepts.png",
        (73, 45, 31),
        (244, 222, 188),
    )


if __name__ == "__main__":
    main()

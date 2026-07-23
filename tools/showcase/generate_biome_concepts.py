#!/usr/bin/env python3
"""Generate standalone Verdant and Ash vegetation concept documents.

These are Canopy-native art-direction candidates.  They intentionally live
under samples/concepts and are not registered with World Studio.
"""

from __future__ import annotations

import json
import os
import sys
from typing import Any

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from generate_textures import bark_height, ensure_textures


REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ROOT = os.path.join(REPO, "samples", "concepts", "documents")


def uuid(index: int) -> str:
    return f"{index:08x}-a711-4b22-9333-{index:012x}"


def curve(keys: list[list[float]]) -> dict[str, Any]:
    return {"interpolation": "linear", "keys": keys}


def material(
    index: int,
    name: str,
    color: list[float],
    *,
    texture: str | None,
    two_sided: bool = False,
    card_region: list[float] | None = None,
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "base_color": color,
        "id": uuid(index),
        "name": name,
        "two_sided": two_sided,
    }
    if texture is not None:
        result["textures"] = {"base_color": texture}
    if card_region is not None:
        result["card_region"] = card_region
    return result


def branch(
    index: int,
    name: str,
    parent: int,
    *,
    mode: str,
    count: int | None = None,
    spacing: float | None = None,
    density: float | None = None,
    first: float = 0.0,
    last: float = 1.0,
    angle: float = 0.0,
    angle_variance: float = 0.0,
    phase: float | None = None,
    azimuth_variance: float | None = None,
    length: float,
    length_variance: float = 0.0,
    radius: float,
    radius_variance: float = 0.0,
    bend: float = 0.0,
    wander: float = 0.0,
    flare: float = 0.0,
    flare_length: float = 0.1,
    flare_lobes: int | None = None,
    flare_lobe: float | None = None,
    radial_segments: int = 7,
    child_profile: list[list[float]] | None = None,
    radius_profile: list[list[float]] | None = None,
    ground_level: float | None = None,
    bark_material: int = 0xB100,
) -> dict[str, Any]:
    props: dict[str, Any] = {
        "generation.mode": mode,
        "generation.first": first,
        "generation.last": last,
        "generation.angle.degrees": angle,
        "generation.angle.variance.degrees": angle_variance,
        "spine.length.absolute": length,
        "spine.length.variance.relative": length_variance,
        "spine.radius.absolute": radius,
        "spine.radius.variance.relative": radius_variance,
        "spine.bend.degrees": bend,
        "spine.wander.degrees": wander,
        "spine.flare.relative": flare,
        "spine.flare.length.relative": flare_length,
        "mesh.radial_segments": radial_segments,
        "mesh.uv.random_phase": True,
        "material.bark": uuid(bark_material),
    }
    if count is not None:
        props["generation.count"] = count
    if spacing is not None:
        props["generation.spacing.relative"] = spacing
    if density is not None:
        props["generation.density.per_meter"] = density
    if phase is not None:
        props["generation.phase.degrees"] = phase
    if azimuth_variance is not None:
        props["generation.azimuth.variance.degrees"] = azimuth_variance
    if flare_lobes is not None:
        props["spine.flare.lobes"] = flare_lobes
    if flare_lobe is not None:
        props["spine.flare.lobe.relative"] = flare_lobe
    if child_profile is not None:
        props["child.length.profile"] = curve(child_profile)
    if radius_profile is not None:
        props["spine.radius.profile"] = curve(radius_profile)
    if ground_level is not None:
        props["spine.ground.level"] = ground_level
    return {
        "id": uuid(index),
        "name": name,
        "parent": uuid(parent),
        "props": props,
        "type": "canopy.branch",
    }


def leaves(
    index: int,
    name: str,
    parent: int,
    material_index: int,
    *,
    spacing: float,
    first: float,
    last: float = 1.0,
    per_point: int,
    length: float,
    width: float,
    size_variance: float = 0.25,
    pitch: float = 48.0,
    pitch_variance: float = 24.0,
    droop: float = 0.2,
    fold: float = 12.0,
) -> dict[str, Any]:
    return {
        "id": uuid(index),
        "name": name,
        "parent": uuid(parent),
        "props": {
            "generation.first": first,
            "generation.last": last,
            "generation.leaves_per_point": per_point,
            "generation.spacing.relative": spacing,
            "leaf.droop.relative": droop,
            "leaf.fold.degrees": fold,
            "leaf.length.absolute": length,
            "leaf.pitch.degrees": pitch,
            "leaf.pitch.variance.degrees": pitch_variance,
            "leaf.size.variance.relative": size_variance,
            "leaf.width.relative": width,
            "material.leaf": uuid(material_index),
            "season.drop.start": 1.0,
        },
        "type": "canopy.batched_leaf",
    }


def fronds(
    index: int,
    name: str,
    parent: int,
    material_index: int,
    *,
    count: int,
    first: float,
    last: float,
    angle: float,
    angle_variance: float,
    length: float,
    length_variance: float,
    bend: float,
    wander: float,
    width: float,
    fold: float,
    serration_count: int,
    serration_depth: float,
    twist: float,
) -> dict[str, Any]:
    return {
        "id": uuid(index),
        "name": name,
        "parent": uuid(parent),
        "props": {
            "frond.fold.degrees": fold,
            "frond.serration.count": serration_count,
            "frond.serration.depth": serration_depth,
            "frond.twist.degrees": twist,
            "frond.width.absolute": width,
            "generation.angle.degrees": angle,
            "generation.angle.variance.degrees": angle_variance,
            "generation.count": count,
            "generation.first": first,
            "generation.last": last,
            "generation.mode": "absolute",
            "material.frond": uuid(material_index),
            "spine.bend.degrees": bend,
            "spine.length.absolute": length,
            "spine.length.variance.relative": length_variance,
            "spine.wander.degrees": wander,
        },
        "type": "canopy.frond",
    }


def roots(
    index: int,
    parent: int,
    *,
    count: int,
    length: float,
    radius: float,
    bark_material: int = 0xB100,
) -> dict[str, Any]:
    return branch(
        index,
        "Surface roots",
        parent,
        mode="absolute",
        count=count,
        first=0.002,
        last=0.018,
        angle=108.0,
        angle_variance=7.0,
        length=length,
        length_variance=0.25,
        radius=radius,
        radius_variance=0.2,
        bend=8.0,
        wander=12.0,
        flare=0.5,
        flare_length=0.25,
        radial_segments=8,
        radius_profile=[[0.0, 1.0], [1.0, 0.03]],
        ground_level=0.015,
        bark_material=bark_material,
    )


def _leaf_polygon(
    center_x: float,
    center_y: float,
    length: float,
    width: float,
    angle: float,
) -> list[tuple[float, float]]:
    direction = np.array([np.sin(angle), -np.cos(angle)])
    side = np.array([direction[1], -direction[0]])
    center = np.array([center_x, center_y])
    base = center - direction * length * 0.45
    tip = center + direction * length * 0.55
    return [
        tuple(base),
        tuple(center - side * width * 0.5 - direction * length * 0.08),
        tuple(tip),
        tuple(center + side * width * 0.5 - direction * length * 0.08),
    ]


def _draw_broad_cluster(
    draw: ImageDraw.ImageDraw,
    box: tuple[int, int, int, int],
    colors: tuple[tuple[int, int, int, int], ...],
    variant: int,
) -> None:
    x0, y0, x1, y1 = box
    width, height = x1 - x0, y1 - y0
    twig = colors[-1]
    stem_x = x0 + width * (0.48 + variant * 0.025)
    draw.line(
        [(stem_x, y1 - height * 0.04), (x0 + width * 0.52, y0 + height * 0.12)],
        fill=twig,
        width=max(3, width // 65),
    )
    placements = [
        (0.48, 0.14, 0.28, 0.22, -0.12),
        (0.28, 0.24, 0.31, 0.24, -0.72),
        (0.70, 0.27, 0.31, 0.24, 0.72),
        (0.20, 0.42, 0.33, 0.25, -0.92),
        (0.50, 0.40, 0.31, 0.24, 0.02),
        (0.78, 0.45, 0.34, 0.26, 0.94),
        (0.27, 0.60, 0.34, 0.26, -0.82),
        (0.58, 0.58, 0.32, 0.25, 0.34),
        (0.77, 0.68, 0.33, 0.25, 0.82),
        (0.36, 0.78, 0.32, 0.25, -0.55),
        (0.61, 0.84, 0.31, 0.24, 0.48),
    ]
    for leaf_index, (cx, cy, leaf_length, leaf_width, angle) in enumerate(placements):
        draw.polygon(
            _leaf_polygon(
                x0 + cx * width,
                y0 + cy * height,
                leaf_length * height,
                leaf_width * width,
                angle + variant * 0.05,
            ),
            fill=colors[leaf_index % (len(colors) - 1)],
        )


def _draw_desert_cluster(
    draw: ImageDraw.ImageDraw,
    box: tuple[int, int, int, int],
    colors: tuple[tuple[int, int, int, int], ...],
    variant: int,
) -> None:
    x0, y0, x1, y1 = box
    width, height = x1 - x0, y1 - y0
    origin = np.array((x0 + width * 0.5, y1 - height * 0.04))
    # Sparse xerophyte tuft: a handful of narrow, waxy blades rather than a
    # broadleaf spray.  The transparent gaps are intentional.
    blade_tips = (
        (0.12, 0.34),
        (0.24, 0.16),
        (0.39, 0.08),
        (0.51, 0.03),
        (0.63, 0.1),
        (0.78, 0.18),
        (0.9, 0.38),
    )
    for blade_index, (tip_x, tip_y) in enumerate(blade_tips):
        tip = np.array((x0 + width * tip_x, y0 + height * tip_y))
        direction = tip - origin
        direction /= max(np.linalg.norm(direction), 1e-9)
        side = np.array((direction[1], -direction[0]))
        half_width = width * (0.012 + 0.002 * ((blade_index + variant) % 3))
        shoulder = origin + direction * height * (0.12 + 0.015 * variant)
        draw.polygon(
            [
                tuple(origin - side * half_width),
                tuple(shoulder - side * half_width * 0.7),
                tuple(tip),
                tuple(shoulder + side * half_width * 0.7),
                tuple(origin + side * half_width),
            ],
            fill=colors[blade_index % (len(colors) - 1)],
        )


def write_textures(
    project_dir: str,
    *,
    biome: str,
    bark_rgb: tuple[float, float, float],
    foliage_palettes: tuple[
        tuple[tuple[int, int, int, int], ...],
        tuple[tuple[int, int, int, int], ...],
    ],
) -> None:
    # Generate the normal map with Canopy's established deterministic utility,
    # then replace albedo/atlas with the concept-specific palettes.
    simple_outline = [[0.0, 0.0], [0.35, 0.45], [0.0, 1.0], [-0.35, 0.45]]
    ensure_textures(
        project_dir,
        {"oak": simple_outline, "birch": simple_outline, "willow": simple_outline},
    )
    texture_dir = os.path.join(project_dir, "assets", "textures")

    height = bark_height(512, seed=19 if biome == "verdant" else 31)
    shade = 0.48 + 0.58 * height
    base = np.asarray(bark_rgb, dtype=np.float64)
    albedo = np.clip(shade[..., None] * base[None, None, :] * 255.0, 0, 255)
    Image.fromarray(albedo.astype(np.uint8), "RGB").save(
        os.path.join(texture_dir, "bark.png"),
        optimize=True,
    )

    atlas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    draw = ImageDraw.Draw(atlas)
    boxes = (
        (30, 30, 482, 482),
        (542, 30, 994, 482),
        (30, 542, 482, 994),
        (542, 542, 994, 994),
    )
    for index, box in enumerate(boxes):
        palette = foliage_palettes[index % 2]
        if biome == "verdant":
            _draw_broad_cluster(draw, box, palette, index)
        else:
            _draw_desert_cluster(draw, box, palette, index)
    atlas.save(os.path.join(texture_dir, "leaf_atlas.png"), optimize=True)


def write_project(
    name: str,
    document_index: int,
    seed: int,
    generators: list[dict[str, Any]],
    materials: list[dict[str, Any]],
    *,
    biome: str,
    bark_rgb: tuple[float, float, float],
    foliage_palettes: tuple[
        tuple[tuple[int, int, int, int], ...],
        tuple[tuple[int, int, int, int], ...],
    ],
) -> None:
    directory = os.path.join(ROOT, f"{name}.canopyproj")
    os.makedirs(directory, exist_ok=True)
    manifest = {
        "document_id": uuid(document_index),
        "engine_algorithm_set": "canopy-1",
        "files": {
            "graph": "graph.json",
            "materials": "materials.json",
            "properties": "properties.json",
        },
        "format": "canopy-authoring",
        "handedness": "right",
        "name": name,
        "schema_version": "1.0.0",
        "seed": seed,
        "units": "meter",
        "up_axis": "Y",
    }
    nodes: list[dict[str, Any]] = []
    properties: dict[str, Any] = {}
    for generator in generators:
        nodes.append(
            {
                "enabled": True,
                "id": generator["id"],
                "name": generator["name"],
                "parent": generator.get("parent"),
                "type": generator["type"],
            }
        )
        if generator.get("props"):
            properties[generator["id"]] = generator["props"]
    nodes.sort(key=lambda node: node["id"])
    documents = {
        "graph.json": {"nodes": nodes},
        "manifest.json": manifest,
        "materials.json": {"materials": materials},
        "properties.json": {"generators": dict(sorted(properties.items()))},
    }
    for filename, contents in documents.items():
        with open(os.path.join(directory, filename), "w", encoding="utf-8") as output:
            json.dump(contents, output, indent=2, sort_keys=True)
            output.write("\n")
    write_textures(
        directory,
        biome=biome,
        bark_rgb=bark_rgb,
        foliage_palettes=foliage_palettes,
    )
    print(f"wrote {directory}")


TREE = {"id": uuid(1), "name": "Plant", "parent": None, "type": "canopy.tree"}
BARK_TEXTURE = "assets/textures/bark.png"
LEAF_TEXTURE = "assets/textures/leaf_atlas.png"
OUTER_REGION = [0.02, 0.02, 0.48, 0.48]
INNER_REGION = [0.52, 0.02, 0.98, 0.48]

VERDANT_PALETTES = (
    (
        (72, 126, 52, 255),
        (91, 148, 63, 255),
        (55, 105, 43, 255),
        (65, 80, 38, 255),
    ),
    (
        (45, 91, 42, 255),
        (62, 112, 49, 255),
        (35, 75, 37, 255),
        (52, 67, 35, 255),
    ),
)
ASH_PALETTES = (
    (
        (178, 147, 91, 255),
        (156, 151, 127, 255),
        (151, 103, 65, 255),
        (105, 76, 55, 255),
    ),
    (
        (132, 116, 86, 255),
        (145, 127, 91, 255),
        (120, 78, 56, 255),
        (88, 65, 51, 255),
    ),
)


def concept_materials(
    bark_name: str,
    bark_color: list[float],
    outer_name: str,
    outer_color: list[float],
    inner_name: str,
    inner_color: list[float],
) -> list[dict[str, Any]]:
    return [
        material(0xB100, bark_name, bark_color, texture=BARK_TEXTURE),
        material(
            0xC100,
            outer_name,
            outer_color,
            texture=LEAF_TEXTURE,
            two_sided=True,
            card_region=OUTER_REGION,
        ),
        material(
            0xC101,
            inner_name,
            inner_color,
            texture=LEAF_TEXTURE,
            two_sided=True,
            card_region=INNER_REGION,
        ),
    ]


# Verdant major — broad layered crown and monumental buttressed trunk.
verdant_major = [
    TREE,
    branch(
        2,
        "Cathedral trunk",
        1,
        mode="absolute",
        count=1,
        length=7.4,
        radius=0.78,
        bend=7.0,
        wander=4.0,
        flare=0.75,
        flare_length=0.22,
        flare_lobes=8,
        flare_lobe=0.24,
        radial_segments=20,
        radius_profile=[[0.0, 1.0], [0.18, 0.76], [0.8, 0.42], [1.0, 0.06]],
    ),
    roots(3, 2, count=8, length=2.5, radius=0.24),
    branch(
        4,
        "Crown scaffold",
        2,
        mode="interval",
        spacing=0.075,
        first=0.42,
        last=0.98,
        angle=62.0,
        angle_variance=15.0,
        length=4.4,
        length_variance=0.2,
        radius=0.18,
        radius_variance=0.18,
        bend=22.0,
        wander=9.0,
        flare=0.32,
        flare_length=0.07,
        radial_segments=11,
        child_profile=[[0.0, 0.82], [0.45, 1.0], [1.0, 0.68]],
        radius_profile=[[0.0, 1.0], [1.0, 0.16]],
    ),
    branch(
        5,
        "Layering branches",
        4,
        mode="interval",
        spacing=0.15,
        first=0.18,
        last=0.98,
        angle=49.0,
        angle_variance=24.0,
        length=2.05,
        length_variance=0.26,
        radius=0.062,
        radius_variance=0.2,
        bend=18.0,
        wander=14.0,
        radial_segments=7,
        child_profile=[[0.0, 0.65], [0.55, 1.0], [1.0, 0.58]],
        radius_profile=[[0.0, 1.0], [1.0, 0.16]],
    ),
    branch(
        6,
        "Canopy twigs",
        5,
        mode="interval",
        spacing=0.22,
        first=0.15,
        last=1.0,
        angle=55.0,
        angle_variance=28.0,
        length=0.85,
        length_variance=0.32,
        radius=0.018,
        radius_variance=0.28,
        bend=12.0,
        wander=17.0,
        radial_segments=5,
        child_profile=[[0.0, 0.72], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.12]],
    ),
    leaves(
        7,
        "Outer cluster cards",
        6,
        0xC100,
        spacing=0.1,
        first=0.12,
        per_point=4,
        length=0.58,
        width=0.92,
        pitch=52.0,
        pitch_variance=32.0,
        droop=0.24,
        fold=16.0,
    ),
    leaves(
        8,
        "Interior cluster cards",
        5,
        0xC101,
        spacing=0.13,
        first=0.28,
        per_point=3,
        length=0.6,
        width=0.94,
        pitch=58.0,
        pitch_variance=30.0,
        droop=0.2,
        fold=14.0,
    ),
]

# Verdant minor — a sturdy single trunk with a compact, readable dome.
verdant_minor = [
    TREE,
    branch(
        2,
        "Lantern trunk",
        1,
        mode="absolute",
        count=1,
        length=4.8,
        radius=0.43,
        bend=8.0,
        wander=5.0,
        flare=0.62,
        flare_length=0.2,
        flare_lobes=6,
        flare_lobe=0.18,
        radial_segments=16,
        radius_profile=[[0.0, 1.0], [0.22, 0.72], [1.0, 0.06]],
    ),
    roots(3, 2, count=6, length=1.45, radius=0.14),
    branch(
        4,
        "Dome limbs",
        2,
        mode="interval",
        spacing=0.09,
        first=0.38,
        last=0.98,
        angle=57.0,
        angle_variance=16.0,
        length=2.55,
        length_variance=0.22,
        radius=0.105,
        radius_variance=0.18,
        bend=19.0,
        wander=9.0,
        radial_segments=9,
        child_profile=[[0.0, 0.75], [0.55, 1.0], [1.0, 0.58]],
        radius_profile=[[0.0, 1.0], [1.0, 0.18]],
    ),
    branch(
        5,
        "Dome twigs",
        4,
        mode="interval",
        spacing=0.18,
        first=0.18,
        last=1.0,
        angle=52.0,
        angle_variance=24.0,
        length=1.05,
        length_variance=0.28,
        radius=0.027,
        radius_variance=0.25,
        bend=14.0,
        wander=14.0,
        radial_segments=6,
        child_profile=[[0.0, 0.72], [1.0, 0.92]],
        radius_profile=[[0.0, 1.0], [1.0, 0.14]],
    ),
    leaves(
        6,
        "Dome cluster cards",
        5,
        0xC100,
        spacing=0.09,
        first=0.12,
        per_point=4,
        length=0.5,
        width=0.94,
        pitch=50.0,
        pitch_variance=30.0,
        droop=0.2,
        fold=14.0,
    ),
    leaves(
        7,
        "Dome interior cards",
        4,
        0xC101,
        spacing=0.12,
        first=0.42,
        per_point=3,
        length=0.52,
        width=0.95,
        pitch=55.0,
        pitch_variance=26.0,
        droop=0.18,
        fold=12.0,
    ),
]

# Verdant shrub — a broad multi-stem woodland thicket.
verdant_shrub = [
    TREE,
    branch(
        2,
        "Thicket stems",
        1,
        mode="absolute",
        count=9,
        angle=16.0,
        angle_variance=10.0,
        length=2.05,
        length_variance=0.28,
        radius=0.075,
        radius_variance=0.24,
        bend=18.0,
        wander=13.0,
        flare=0.5,
        flare_length=0.18,
        radial_segments=8,
        child_profile=[[0.0, 0.78], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.2]],
    ),
    branch(
        3,
        "Thicket sprays",
        2,
        mode="interval",
        spacing=0.2,
        first=0.18,
        last=1.0,
        angle=54.0,
        angle_variance=25.0,
        length=0.82,
        length_variance=0.32,
        radius=0.021,
        radius_variance=0.25,
        bend=12.0,
        wander=18.0,
        radial_segments=5,
        child_profile=[[0.0, 0.65], [0.7, 1.0], [1.0, 0.8]],
        radius_profile=[[0.0, 1.0], [1.0, 0.12]],
    ),
    leaves(
        4,
        "Thicket cluster cards",
        3,
        0xC100,
        spacing=0.1,
        first=0.1,
        per_point=3,
        length=0.31,
        width=0.95,
        pitch=48.0,
        pitch_variance=34.0,
        droop=0.25,
        fold=14.0,
    ),
    leaves(
        5,
        "Thicket core cards",
        2,
        0xC101,
        spacing=0.12,
        first=0.4,
        per_point=2,
        length=0.35,
        width=0.95,
        pitch=58.0,
        pitch_variance=28.0,
        droop=0.22,
        fold=12.0,
    ),
]

# Ash major — massive water-storing trunk and a broad, shaded desert umbrella.
ash_major = [
    TREE,
    branch(
        2,
        "Ember bottle trunk",
        1,
        mode="absolute",
        count=1,
        length=5.5,
        radius=0.88,
        bend=5.0,
        wander=4.0,
        flare=0.72,
        flare_length=0.28,
        flare_lobes=7,
        flare_lobe=0.2,
        radial_segments=20,
        radius_profile=[[0.0, 1.0], [0.28, 0.86], [0.66, 0.5], [1.0, 0.07]],
    ),
    roots(3, 2, count=7, length=2.2, radius=0.22),
    branch(
        4,
        "Umbrella scaffold",
        2,
        mode="absolute",
        count=9,
        first=0.5,
        last=0.98,
        angle=73.0,
        angle_variance=12.0,
        length=5.5,
        length_variance=0.16,
        radius=0.19,
        radius_variance=0.17,
        bend=-12.0,
        wander=10.0,
        radial_segments=11,
        child_profile=[[0.0, 0.8], [0.65, 1.0], [1.0, 0.72]],
        radius_profile=[[0.0, 1.0], [1.0, 0.15]],
    ),
    branch(
        5,
        "Flat crown branches",
        4,
        mode="interval",
        spacing=0.23,
        first=0.22,
        last=1.0,
        angle=48.0,
        angle_variance=20.0,
        length=1.85,
        length_variance=0.25,
        radius=0.052,
        radius_variance=0.22,
        bend=-8.0,
        wander=15.0,
        radial_segments=7,
        child_profile=[[0.0, 0.64], [0.62, 1.0], [1.0, 0.8]],
        radius_profile=[[0.0, 1.0], [1.0, 0.14]],
    ),
    branch(
        6,
        "Dry crown twigs",
        5,
        mode="interval",
        spacing=0.33,
        first=0.18,
        last=1.0,
        angle=58.0,
        angle_variance=25.0,
        length=0.68,
        length_variance=0.3,
        radius=0.015,
        radius_variance=0.3,
        bend=5.0,
        wander=19.0,
        radial_segments=5,
        child_profile=[[0.0, 0.7], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.1]],
    ),
    leaves(
        7,
        "Sunlit needle tufts",
        6,
        0xC100,
        spacing=0.28,
        first=0.52,
        per_point=2,
        length=0.68,
        width=0.72,
        pitch=68.0,
        pitch_variance=22.0,
        droop=0.08,
        fold=7.0,
    ),
    leaves(
        8,
        "Shaded needle tufts",
        5,
        0xC101,
        spacing=0.38,
        first=0.72,
        per_point=1,
        length=0.74,
        width=0.74,
        pitch=72.0,
        pitch_variance=20.0,
        droop=0.06,
        fold=6.0,
    ),
]

# Ash minor — a twisted mesquite-like survivor with a wide asymmetric crown.
ash_minor = [
    TREE,
    branch(
        2,
        "Dustwood trunk",
        1,
        mode="absolute",
        count=1,
        length=3.9,
        radius=0.42,
        bend=23.0,
        wander=12.0,
        flare=0.68,
        flare_length=0.24,
        flare_lobes=6,
        flare_lobe=0.22,
        radial_segments=16,
        radius_profile=[[0.0, 1.0], [0.28, 0.7], [1.0, 0.07]],
    ),
    roots(3, 2, count=6, length=1.6, radius=0.14),
    branch(
        4,
        "Twisted forks",
        2,
        mode="bifurcation",
        count=5,
        first=0.68,
        last=0.98,
        angle=47.0,
        angle_variance=12.0,
        phase=18.0,
        azimuth_variance=18.0,
        length=2.35,
        length_variance=0.2,
        radius=0.105,
        radius_variance=0.18,
        bend=-16.0,
        wander=16.0,
        radial_segments=9,
        child_profile=[[0.0, 0.82], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.18]],
    ),
    branch(
        5,
        "Mesquite sprays",
        4,
        mode="interval",
        spacing=0.25,
        first=0.2,
        last=1.0,
        angle=56.0,
        angle_variance=24.0,
        length=1.0,
        length_variance=0.28,
        radius=0.028,
        radius_variance=0.25,
        bend=-5.0,
        wander=18.0,
        radial_segments=6,
        child_profile=[[0.0, 0.62], [0.7, 1.0], [1.0, 0.82]],
        radius_profile=[[0.0, 1.0], [1.0, 0.12]],
    ),
    leaves(
        6,
        "Dustwood needle tufts",
        5,
        0xC100,
        spacing=0.26,
        first=0.48,
        per_point=2,
        length=0.6,
        width=0.72,
        pitch=66.0,
        pitch_variance=24.0,
        droop=0.1,
        fold=7.0,
    ),
    leaves(
        7,
        "Dustwood inner tufts",
        4,
        0xC101,
        spacing=0.4,
        first=0.75,
        per_point=1,
        length=0.66,
        width=0.74,
        pitch=70.0,
        pitch_variance=22.0,
        droop=0.08,
        fold=6.0,
    ),
]

# Ash shrub — a low spear-leaved rosette inspired by agave and dry yucca.
ash_shrub = [
    TREE,
    branch(
        2,
        "Spearbush crown",
        1,
        mode="absolute",
        count=1,
        length=0.42,
        radius=0.23,
        bend=2.0,
        wander=2.0,
        flare=0.65,
        flare_length=0.5,
        flare_lobes=7,
        flare_lobe=0.18,
        radial_segments=12,
        radius_profile=[[0.0, 1.0], [1.0, 0.62]],
    ),
    fronds(
        3,
        "Outer spear leaves",
        2,
        0xC102,
        count=18,
        first=0.62,
        last=1.0,
        angle=68.0,
        angle_variance=9.0,
        length=1.35,
        length_variance=0.16,
        bend=18.0,
        wander=2.0,
        width=0.12,
        fold=58.0,
        serration_count=7,
        serration_depth=0.12,
        twist=4.0,
    ),
    fronds(
        4,
        "Inner spear leaves",
        2,
        0xC103,
        count=14,
        first=0.72,
        last=1.0,
        angle=30.0,
        angle_variance=10.0,
        length=1.5,
        length_variance=0.14,
        bend=-9.0,
        wander=2.0,
        width=0.1,
        fold=62.0,
        serration_count=5,
        serration_depth=0.09,
        twist=3.0,
    ),
]


def main() -> None:
    verdant_materials = concept_materials(
        "verdant_grooved_bark",
        [0.34, 0.29, 0.22, 1.0],
        "verdant_sun_clusters",
        [0.33, 0.55, 0.22, 1.0],
        "verdant_shade_clusters",
        [0.18, 0.36, 0.19, 1.0],
    )
    ash_materials = concept_materials(
        "ash_dust_bark",
        [0.46, 0.27, 0.18, 1.0],
        "ash_sun_needles",
        [0.68, 0.54, 0.31, 1.0],
        "ash_shade_needles",
        [0.5, 0.42, 0.3, 1.0],
    )
    ash_materials.extend(
        [
            material(
                0xC102,
                "ash_outer_spear_blades",
                [0.62, 0.46, 0.27, 1.0],
                texture=None,
                two_sided=True,
            ),
            material(
                0xC103,
                "ash_inner_spear_blades",
                [0.5, 0.47, 0.38, 1.0],
                texture=None,
                two_sided=True,
            ),
        ]
    )

    write_project(
        "VerdantCathedral",
        0xD101,
        0x51A771,
        verdant_major,
        verdant_materials,
        biome="verdant",
        bark_rgb=(0.34, 0.29, 0.22),
        foliage_palettes=VERDANT_PALETTES,
    )
    write_project(
        "VerdantLantern",
        0xD102,
        0x51A772,
        verdant_minor,
        verdant_materials,
        biome="verdant",
        bark_rgb=(0.37, 0.31, 0.23),
        foliage_palettes=VERDANT_PALETTES,
    )
    write_project(
        "VerdantThicket",
        0xD103,
        0x51A773,
        verdant_shrub,
        verdant_materials,
        biome="verdant",
        bark_rgb=(0.31, 0.27, 0.2),
        foliage_palettes=VERDANT_PALETTES,
    )
    write_project(
        "AshEmberCrown",
        0xD201,
        0xA55101,
        ash_major,
        ash_materials,
        biome="ash",
        bark_rgb=(0.46, 0.27, 0.18),
        foliage_palettes=ASH_PALETTES,
    )
    write_project(
        "AshDustwood",
        0xD202,
        0xA55102,
        ash_minor,
        ash_materials,
        biome="ash",
        bark_rgb=(0.42, 0.24, 0.16),
        foliage_palettes=ASH_PALETTES,
    )
    write_project(
        "AshSpearbush",
        0xD203,
        0xA55103,
        ash_shrub,
        ash_materials,
        biome="ash",
        bark_rgb=(0.39, 0.23, 0.16),
        foliage_palettes=ASH_PALETTES,
    )


if __name__ == "__main__":
    main()

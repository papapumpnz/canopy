#!/usr/bin/env python3
"""Generate the stylised low-poly cartoon tree concept documents.

Seven Canopy-native art-direction candidates in one cartoon family: three
large canopied trees, two medium and two small.  They live under
samples/cartoon and are deliberately not registered with World Studio.

The look comes from three deliberate choices:

  * chunky proportions — short, very thick, heavily flared trunks with a
    slow taper, so the silhouette reads as a cartoon tree rather than a
    scaled-down realistic one;
  * few, very large foliage cards — every leaf material carries a
    `card_region`, which is the two-triangle game-budget path in the
    evaluator, and each card shows a whole hand-painted leaf blob.  A full
    crown costs a few hundred triangles instead of tens of thousands of
    per-leaf quads;
  * low radial segment counts — trunks at 6, boughs at 5, twigs at 4.  The
    faceting is visible on purpose.

Radial resolution is authored in the document rather than left to the
export profile, so these read as low-poly at every profile; the draft
profile additionally thins the ring count along each spine.  See
`tools/showcase/presets/obj-cartoon.json`.
"""

from __future__ import annotations

import json
import math
import os
from typing import Any

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ROOT = os.path.join(REPO, "samples", "cartoon", "documents")


def uuid(index: int) -> str:
    return f"{index:08x}-c470-4f00-9111-{index:012x}"


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
    radial_segments: int = 5,
    child_profile: list[list[float]] | None = None,
    radius_profile: list[list[float]] | None = None,
    ground_level: float | None = None,
    bark_material: int = 0xB200,
) -> dict[str, Any]:
    """A branch generator.

    Flare lobes are intentionally unused across this family: lobes alias
    badly against a 5–6 segment ring, so the buttress character comes from
    a smooth `spine.flare.relative` plus explicit chunky root branches.
    `generation.azimuth.variance.degrees` is only emitted for bifurcation,
    the one mode here that supplies its own azimuth — elsewhere the
    evaluator draws azimuth randomly and the key would be dead weight.
    """
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
    if phase is not None:
        props["generation.phase.degrees"] = phase
    if azimuth_variance is not None:
        props["generation.azimuth.variance.degrees"] = azimuth_variance
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
    size_variance: float = 0.22,
    pitch: float = 40.0,
    pitch_variance: float = 42.0,
    droop: float = 0.12,
) -> dict[str, Any]:
    """A batched foliage-card generator.

    `leaf.fold.degrees` is deliberately absent: every material in this
    family sets `card_region`, and the evaluator ignores fold on the
    flat two-triangle card path.
    """
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


def roots(index: int, parent: int, *, count: int, length: float, radius: float) -> dict[str, Any]:
    """Chunky cartoon surface roots — few, fat and short."""
    return branch(
        index,
        "Root toes",
        parent,
        mode="absolute",
        count=count,
        first=0.002,
        last=0.014,
        angle=104.0,
        angle_variance=5.0,
        length=length,
        length_variance=0.18,
        radius=radius,
        radius_variance=0.14,
        bend=14.0,
        wander=6.0,
        flare=0.7,
        flare_length=0.3,
        radial_segments=4,
        radius_profile=[[0.0, 1.0], [1.0, 0.08]],
        ground_level=0.02,
    )


# ---------------------------------------------------------------------------
# Cartoon texture synthesis
# ---------------------------------------------------------------------------


def _blob_circles(variant: int) -> list[tuple[float, float, float]]:
    """A lumpy cloud silhouette as overlapping unit-space circles.

    Circles sit on a squashed ring around a fat core, which gives the
    scalloped 'bunch of leaves' outline that cartoon foliage relies on
    while staying one solid mass, so the alpha mask has no interior holes.
    """
    lumps = 7 + (variant % 2)
    circles: list[tuple[float, float, float]] = [(0.5, 0.54, 0.27)]
    for i in range(lumps):
        angle = 2.0 * math.pi * (i / lumps) + 0.35 * variant
        wobble = 0.035 * math.sin(3.0 * angle + variant)
        circles.append(
            (
                0.5 + (0.26 + wobble) * math.cos(angle),
                0.52 - (0.22 + wobble) * math.sin(angle),
                0.175 + 0.03 * math.cos(2.0 * angle + variant),
            )
        )
    return circles


def _draw_toon_cluster(
    image: Image.Image,
    box: tuple[int, int, int, int],
    palette: dict[str, tuple[int, int, int, int]],
    variant: int,
) -> None:
    """Paint one flat-shaded foliage blob with a hard toon rim."""
    x0, y0, x1, y1 = box
    width, height = x1 - x0, y1 - y0
    circles = _blob_circles(variant)
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    def ellipse(circle: tuple[float, float, float], grow: float, fill) -> None:
        cx, cy, r = circle
        rx, ry = (r + grow) * width, (r + grow) * height
        draw.ellipse(
            (
                x0 + cx * width - rx,
                y0 + cy * height - ry,
                x0 + cx * width + rx,
                y0 + cy * height + ry,
            ),
            fill=fill,
        )

    # 1. Rim: the silhouette grown outwards in the darkest tone.
    for circle in circles:
        ellipse(circle, 0.022, palette["rim"])
    # 2. Body: one flat mid tone, no gradient.
    for circle in circles:
        ellipse(circle, 0.0, palette["body"])
    # 3. Occlusion lobe along the lower edge.
    for cx, cy, r in circles:
        if cy > 0.5:
            ellipse((cx, cy + 0.035, r * 0.82), 0.0, palette["shade"])
    # 4. Sun-side highlight lumps, up and to the left.
    for index, (cx, cy, r) in enumerate(circles):
        if cy < 0.56 and index % 2 == 0:
            ellipse((cx - 0.035, cy - 0.05, r * 0.62), 0.0, palette["light"])
    # 5. A few specular pops to finish the toon ramp.
    for index, (cx, cy, r) in enumerate(circles):
        if cy < 0.42 and index % 3 == 0:
            ellipse((cx - 0.05, cy - 0.07, r * 0.3), 0.0, palette["spec"])

    image.alpha_composite(layer)


def cartoon_leaf_atlas(path: str, sun: dict, shade: dict) -> None:
    """1024² foliage atlas.

    The left column carries the sun palette and the right column the shade
    palette in BOTH rows, so the two card regions resolve to the intended
    palette whichever way round the exporter orients V.
    """
    atlas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    for box, palette, variant in (
        ((16, 16, 496, 496), sun, 0),
        ((528, 16, 1008, 496), shade, 1),
        ((16, 528, 496, 1008), sun, 0),
        ((528, 528, 1008, 1008), shade, 1),
    ):
        _draw_toon_cluster(atlas, box, palette, variant)
    atlas.save(path, optimize=True)


def cartoon_bark(path: str, base_rgb: tuple[float, float, float], seed: int) -> None:
    """Flat stylised bark: a few soft vertical bands, no photographic noise.

    Deliberately low contrast.  The cartoon read comes from the silhouette
    and the flat foliage, so the trunk stays a calm block of colour.
    """
    size = 512
    rng = np.random.default_rng(seed)
    x = np.linspace(0.0, 1.0, size, endpoint=False)
    shade = np.zeros(size)
    for _ in range(7):
        centre = float(rng.uniform(0.0, 1.0))
        width = float(rng.uniform(0.03, 0.09))
        depth = float(rng.uniform(0.06, 0.16))
        delta = np.minimum(np.abs(x - centre), 1.0 - np.abs(x - centre))
        shade -= depth * np.exp(-((delta / width) ** 2))
    shade = shade[None, :].repeat(size, axis=0)
    # Gentle horizontal drift so the bands are not perfectly straight.
    drift = (np.sin(np.linspace(0.0, 2.0 * math.pi, size)) * 3.0).astype(int)[:, None]
    columns = (np.arange(size)[None, :] + drift) % size
    shade = np.take_along_axis(shade, columns, axis=1)

    base = np.asarray(base_rgb, dtype=np.float64)
    albedo = np.clip((1.0 + shade)[..., None] * base[None, None, :] * 255.0, 0, 255)
    image = Image.fromarray(albedo.astype(np.uint8), "RGB")
    image = image.filter(ImageFilter.GaussianBlur(1.4))
    image.save(path, optimize=True)


# ---------------------------------------------------------------------------
# Document assembly
# ---------------------------------------------------------------------------


def write_project(
    name: str,
    document_index: int,
    seed: int,
    generators: list[dict[str, Any]],
    materials: list[dict[str, Any]],
    *,
    bark_rgb: tuple[float, float, float],
) -> None:
    directory = os.path.join(ROOT, f"{name}.canopyproj")
    texture_dir = os.path.join(directory, "assets", "textures")
    os.makedirs(texture_dir, exist_ok=True)
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
    cartoon_bark(os.path.join(texture_dir, "bark.png"), bark_rgb, seed & 0xFFFF)
    cartoon_leaf_atlas(os.path.join(texture_dir, "leaf_atlas.png"), SUN_PALETTE, SHADE_PALETTE)
    print(f"wrote {directory}")


TREE = {"id": uuid(1), "name": "Plant", "parent": None, "type": "canopy.tree"}
BARK_TEXTURE = "assets/textures/bark.png"
LEAF_TEXTURE = "assets/textures/leaf_atlas.png"
SUN_REGION = [0.02, 0.02, 0.48, 0.48]
SHADE_REGION = [0.52, 0.02, 0.98, 0.48]

# Saturated, high-value greens: cartoon foliage is lit like paint, so the
# ramp steps in clear bands rather than blending.  These are pitched bright
# because the diagnostic rasterizer multiplies them by a 0.32–1.0 lambert
# term; a palette that looks correct flat renders as olive sludge.
SUN_PALETTE = {
    "rim": (40, 92, 40, 255),
    "shade": (74, 146, 56, 255),
    "body": (112, 190, 70, 255),
    "light": (152, 216, 96, 255),
    "spec": (196, 236, 132, 255),
}
SHADE_PALETTE = {
    "rim": (28, 68, 32, 255),
    "shade": (48, 106, 44, 255),
    "body": (76, 148, 58, 255),
    "light": (104, 178, 70, 255),
    "spec": (138, 202, 92, 255),
}

CARTOON_MATERIALS = [
    material(0xB200, "toon_bark", [0.68, 0.49, 0.32, 1.0], texture=BARK_TEXTURE),
    material(
        0xC200,
        "toon_canopy_sun",
        [0.44, 0.75, 0.27, 1.0],
        texture=LEAF_TEXTURE,
        two_sided=True,
        card_region=SUN_REGION,
    ),
    material(
        0xC201,
        "toon_canopy_shade",
        [0.30, 0.58, 0.23, 1.0],
        texture=LEAF_TEXTURE,
        two_sided=True,
        card_region=SHADE_REGION,
    ),
]


# --- Large 1: the hero. Fat trunk, one enormous rounded canopy. ------------
big_top = [
    TREE,
    branch(
        2,
        "Fat trunk",
        1,
        mode="absolute",
        count=1,
        length=4.8,
        radius=0.62,
        bend=6.0,
        wander=3.0,
        flare=1.05,
        flare_length=0.28,
        radial_segments=6,
        radius_profile=[[0.0, 1.0], [0.3, 0.68], [0.82, 0.5], [1.0, 0.36]],
    ),
    roots(3, 2, count=5, length=1.15, radius=0.24),
    branch(
        4,
        "Main boughs",
        2,
        mode="absolute",
        count=8,
        first=0.66,
        last=0.96,
        angle=46.0,
        angle_variance=9.0,
        length=2.4,
        length_variance=0.14,
        radius=0.2,
        radius_variance=0.12,
        bend=40.0,
        wander=5.0,
        flare=0.42,
        flare_length=0.1,
        radial_segments=5,
        child_profile=[[0.0, 0.85], [0.55, 1.0], [1.0, 0.8]],
        radius_profile=[[0.0, 1.0], [1.0, 0.3]],
    ),
    branch(
        5,
        "Crown forks",
        4,
        mode="absolute",
        count=4,
        first=0.42,
        last=0.95,
        angle=38.0,
        angle_variance=14.0,
        length=1.3,
        length_variance=0.22,
        radius=0.078,
        radius_variance=0.18,
        bend=24.0,
        wander=10.0,
        radial_segments=4,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.26]],
    ),
    leaves(
        6,
        "Canopy sun cards",
        5,
        0xC200,
        spacing=0.2,
        first=0.08,
        per_point=3,
        length=1.35,
        width=1.06,
        pitch=42.0,
        pitch_variance=26.0,
        droop=0.1,
    ),
    leaves(
        7,
        "Canopy shade cards",
        4,
        0xC201,
        spacing=0.24,
        first=0.42,
        per_point=2,
        length=1.5,
        width=1.02,
        pitch=50.0,
        pitch_variance=24.0,
        droop=0.08,
    ),
]

# --- Large 2: wide spreading twin-fork with a broad flattened canopy. ------
broad_spread = [
    TREE,
    branch(
        2,
        "Squat trunk",
        1,
        mode="absolute",
        count=1,
        length=2.9,
        radius=0.66,
        bend=9.0,
        wander=4.0,
        flare=1.15,
        flare_length=0.34,
        radial_segments=6,
        radius_profile=[[0.0, 1.0], [0.34, 0.72], [1.0, 0.46]],
    ),
    roots(3, 2, count=6, length=1.3, radius=0.26),
    branch(
        4,
        "Twin forks",
        2,
        mode="bifurcation",
        count=3,
        first=0.72,
        last=0.97,
        angle=40.0,
        angle_variance=8.0,
        phase=26.0,
        azimuth_variance=16.0,
        length=2.35,
        length_variance=0.16,
        radius=0.27,
        radius_variance=0.1,
        bend=26.0,
        wander=7.0,
        flare=0.4,
        flare_length=0.12,
        radial_segments=5,
        child_profile=[[0.0, 0.88], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.34]],
    ),
    branch(
        5,
        "Spread twigs",
        4,
        mode="absolute",
        count=4,
        first=0.34,
        last=0.96,
        angle=46.0,
        angle_variance=16.0,
        length=1.2,
        length_variance=0.24,
        radius=0.07,
        radius_variance=0.2,
        bend=16.0,
        wander=12.0,
        radial_segments=4,
        child_profile=[[0.0, 0.78], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.24]],
    ),
    leaves(
        6,
        "Spread sun cards",
        5,
        0xC200,
        spacing=0.22,
        first=0.08,
        per_point=3,
        length=1.3,
        width=1.1,
        pitch=34.0,
        pitch_variance=26.0,
        droop=0.1,
    ),
    leaves(
        7,
        "Spread shade cards",
        4,
        0xC201,
        spacing=0.26,
        first=0.4,
        per_point=2,
        length=1.45,
        width=1.05,
        pitch=42.0,
        pitch_variance=24.0,
        droop=0.1,
    ),
]

# --- Large 3: tall storybook tree with a stacked cloud-tier canopy. --------
tall_stack = [
    TREE,
    branch(
        2,
        "Tall trunk",
        1,
        mode="absolute",
        count=1,
        length=6.1,
        radius=0.58,
        bend=8.0,
        wander=5.0,
        flare=1.0,
        flare_length=0.24,
        radial_segments=6,
        radius_profile=[[0.0, 1.0], [0.26, 0.66], [0.7, 0.4], [1.0, 0.1]],
    ),
    roots(3, 2, count=5, length=1.1, radius=0.22),
    branch(
        4,
        "Tier boughs",
        2,
        mode="interval",
        spacing=0.1,
        first=0.4,
        last=0.99,
        angle=54.0,
        angle_variance=12.0,
        length=2.0,
        length_variance=0.2,
        radius=0.15,
        radius_variance=0.16,
        bend=30.0,
        wander=8.0,
        flare=0.36,
        flare_length=0.1,
        radial_segments=5,
        # Long at the bottom, short at the top: the tiers stack into a
        # storybook cone instead of a uniform column.
        child_profile=[[0.0, 1.15], [0.55, 0.9], [1.0, 0.42]],
        radius_profile=[[0.0, 1.0], [1.0, 0.28]],
    ),
    branch(
        5,
        "Tier twigs",
        4,
        mode="absolute",
        count=3,
        first=0.45,
        last=0.95,
        angle=40.0,
        angle_variance=16.0,
        length=1.0,
        length_variance=0.24,
        radius=0.055,
        radius_variance=0.2,
        bend=20.0,
        wander=12.0,
        radial_segments=4,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.22]],
    ),
    leaves(
        6,
        "Tier sun cards",
        5,
        0xC200,
        spacing=0.24,
        first=0.08,
        per_point=3,
        length=1.2,
        width=1.08,
        pitch=40.0,
        pitch_variance=26.0,
        droop=0.1,
    ),
    leaves(
        7,
        "Tier shade cards",
        4,
        0xC201,
        spacing=0.26,
        first=0.38,
        per_point=2,
        length=1.35,
        width=1.02,
        pitch=48.0,
        pitch_variance=24.0,
        droop=0.08,
    ),
    # The leader outruns the topmost tier, so it needs its own tuft —
    # without this the bare trunk tip spikes straight out of the crown.
    leaves(
        8,
        "Leader tuft cards",
        2,
        0xC200,
        spacing=0.05,
        first=0.86,
        per_point=3,
        length=1.05,
        width=1.06,
        pitch=44.0,
        pitch_variance=28.0,
        droop=0.1,
    ),
]

# --- Medium 1: stout round-topped tree, the everyday filler. ---------------
round_top = [
    TREE,
    branch(
        2,
        "Stout trunk",
        1,
        mode="absolute",
        count=1,
        length=2.5,
        radius=0.36,
        bend=7.0,
        wander=4.0,
        flare=0.95,
        flare_length=0.3,
        radial_segments=6,
        radius_profile=[[0.0, 1.0], [0.32, 0.7], [1.0, 0.42]],
    ),
    roots(3, 2, count=5, length=0.66, radius=0.13),
    branch(
        4,
        "Round boughs",
        2,
        mode="absolute",
        count=8,
        first=0.55,
        last=0.97,
        angle=42.0,
        angle_variance=11.0,
        length=1.25,
        length_variance=0.18,
        radius=0.115,
        radius_variance=0.14,
        bend=30.0,
        wander=7.0,
        radial_segments=5,
        child_profile=[[0.0, 0.82], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.28]],
    ),
    branch(
        5,
        "Round twigs",
        4,
        mode="absolute",
        count=3,
        first=0.45,
        last=0.95,
        angle=38.0,
        angle_variance=16.0,
        length=0.66,
        length_variance=0.22,
        radius=0.042,
        radius_variance=0.2,
        bend=18.0,
        wander=11.0,
        radial_segments=4,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.24]],
    ),
    leaves(
        6,
        "Round sun cards",
        5,
        0xC200,
        spacing=0.26,
        first=0.08,
        per_point=3,
        length=0.85,
        width=1.08,
        pitch=40.0,
        pitch_variance=26.0,
        droop=0.1,
    ),
    leaves(
        7,
        "Round shade cards",
        4,
        0xC201,
        spacing=0.28,
        first=0.42,
        per_point=2,
        length=0.95,
        width=1.02,
        pitch=48.0,
        pitch_variance=24.0,
        droop=0.08,
    ),
]

# --- Medium 2: leaning character tree with an off-centre crown. ------------
leaner = [
    TREE,
    branch(
        2,
        "Leaning trunk",
        1,
        mode="absolute",
        count=1,
        length=2.8,
        radius=0.33,
        bend=34.0,
        wander=9.0,
        flare=1.0,
        flare_length=0.32,
        radial_segments=6,
        radius_profile=[[0.0, 1.0], [0.3, 0.68], [1.0, 0.4]],
    ),
    roots(3, 2, count=5, length=0.7, radius=0.13),
    branch(
        4,
        "Lean boughs",
        2,
        mode="absolute",
        count=4,
        first=0.6,
        last=0.96,
        angle=46.0,
        angle_variance=14.0,
        length=1.45,
        length_variance=0.22,
        radius=0.105,
        radius_variance=0.16,
        bend=32.0,
        wander=10.0,
        radial_segments=5,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.26]],
    ),
    branch(
        5,
        "Lean twigs",
        4,
        mode="absolute",
        count=3,
        first=0.45,
        last=0.95,
        angle=42.0,
        angle_variance=18.0,
        length=0.7,
        length_variance=0.24,
        radius=0.04,
        radius_variance=0.22,
        bend=18.0,
        wander=14.0,
        radial_segments=4,
        child_profile=[[0.0, 0.78], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.22]],
    ),
    leaves(
        6,
        "Lean sun cards",
        5,
        0xC200,
        spacing=0.26,
        first=0.08,
        per_point=3,
        length=0.82,
        width=1.1,
        pitch=38.0,
        pitch_variance=26.0,
        droop=0.12,
    ),
    leaves(
        7,
        "Lean shade cards",
        4,
        0xC201,
        spacing=0.28,
        first=0.42,
        per_point=2,
        length=0.92,
        width=1.04,
        pitch=46.0,
        pitch_variance=24.0,
        droop=0.1,
    ),
]

# --- Small 1: a young sapling, one clean stem and a small puff. ------------
sapling = [
    TREE,
    branch(
        2,
        "Sapling stem",
        1,
        mode="absolute",
        count=1,
        length=1.35,
        radius=0.13,
        bend=10.0,
        wander=6.0,
        flare=0.8,
        flare_length=0.26,
        radial_segments=5,
        radius_profile=[[0.0, 1.0], [0.35, 0.7], [1.0, 0.4]],
    ),
    branch(
        3,
        "Sapling shoots",
        2,
        mode="absolute",
        count=4,
        first=0.55,
        last=0.96,
        angle=40.0,
        angle_variance=14.0,
        length=0.52,
        length_variance=0.2,
        radius=0.032,
        radius_variance=0.18,
        bend=24.0,
        wander=10.0,
        radial_segments=4,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.24]],
    ),
    leaves(
        4,
        "Sapling sun cards",
        3,
        0xC200,
        spacing=0.3,
        first=0.1,
        per_point=3,
        length=0.5,
        width=1.08,
        pitch=38.0,
        pitch_variance=28.0,
        droop=0.1,
    ),
    leaves(
        5,
        "Sapling shade cards",
        2,
        0xC201,
        spacing=0.32,
        first=0.6,
        per_point=2,
        length=0.54,
        width=1.02,
        pitch=46.0,
        pitch_variance=26.0,
        droop=0.08,
    ),
]

# --- Small 2: a round multi-stem bush for undergrowth dressing. ------------
bush = [
    TREE,
    branch(
        2,
        "Bush stems",
        1,
        mode="absolute",
        count=6,
        angle=17.0,
        angle_variance=8.0,
        length=0.72,
        length_variance=0.22,
        radius=0.055,
        radius_variance=0.2,
        bend=22.0,
        wander=10.0,
        flare=0.75,
        flare_length=0.24,
        radial_segments=5,
        child_profile=[[0.0, 0.82], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.3]],
    ),
    branch(
        3,
        "Bush sprigs",
        2,
        mode="absolute",
        count=3,
        first=0.42,
        last=0.95,
        angle=44.0,
        angle_variance=18.0,
        length=0.42,
        length_variance=0.24,
        radius=0.024,
        radius_variance=0.22,
        bend=18.0,
        wander=14.0,
        radial_segments=4,
        child_profile=[[0.0, 0.8], [1.0, 1.0]],
        radius_profile=[[0.0, 1.0], [1.0, 0.22]],
    ),
    leaves(
        4,
        "Bush sun cards",
        3,
        0xC200,
        spacing=0.3,
        first=0.08,
        per_point=3,
        length=0.44,
        width=1.1,
        pitch=36.0,
        pitch_variance=28.0,
        droop=0.1,
    ),
    leaves(
        5,
        "Bush shade cards",
        2,
        0xC201,
        spacing=0.32,
        first=0.38,
        per_point=2,
        length=0.48,
        width=1.04,
        pitch=44.0,
        pitch_variance=26.0,
        droop=0.08,
    ),
]


# Bark albedo is authored bright and warm for the same reason as the foliage
# palette — the lambert term darkens it substantially, and a cartoon trunk
# should stay a friendly mid-brown rather than going to near-black in shadow.
# Seeds are chosen from a per-species sweep rather than left arbitrary.
# Bough azimuth is drawn randomly (the evaluator offers no azimuth spread
# control outside phyllotaxy/bifurcation), so with only 4–8 boughs the seed
# is what decides whether a crown reads round or lopsided.
CONCEPTS = (
    ("ToonBigTop", 0xD301, 0x2468, big_top, (0.70, 0.50, 0.33)),
    ("ToonBroadSpread", 0xD302, 0x70017A2, broad_spread, (0.66, 0.47, 0.31)),
    ("ToonTallStack", 0xD303, 0x9911, tall_stack, (0.73, 0.53, 0.35)),
    ("ToonRoundTop", 0xD304, 0x9911, round_top, (0.68, 0.49, 0.32)),
    ("ToonLeaner", 0xD305, 0xABCD, leaner, (0.64, 0.45, 0.30)),
    ("ToonSapling", 0xD306, 0x2468, sapling, (0.71, 0.52, 0.34)),
    ("ToonBush", 0xD307, 0x9911, bush, (0.62, 0.44, 0.29)),
)


def main() -> None:
    os.makedirs(ROOT, exist_ok=True)
    for name, document_index, seed, generators, bark_rgb in CONCEPTS:
        write_project(
            name,
            document_index,
            seed,
            generators,
            CARTOON_MATERIALS,
            bark_rgb=bark_rgb,
        )


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate the showcase .canopyproj documents in samples/documents/.

These are the authoritative sources for the reference renders in
samples/renders/. Regeneration pipeline (from the repo root):

    python3 tools/showcase/generate_documents.py
    cmake --build --preset headless-dev
    for t in samples/documents/*.canopyproj; do
        build/headless-dev/apps/cli/canopy-cli export "$t"             --preset tests/presets/obj-diagnostic.json             --out "build/showcase/$(basename "$t" .canopyproj)"
    done
    python3 tools/showcase/render_showcase.py
"""
import json, os, sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ROOT = os.path.join(REPO, "samples", "documents")

def U(n):  # deterministic uuid from an index
    return f"{n:08x}-1111-4222-8333-{n:012x}"

def curve(keys, interp="linear"):
    return {"interpolation": interp, "keys": keys}

def write(name, seed, generators, materials):
    d = os.path.join(ROOT, f"{name}.canopyproj")
    os.makedirs(d, exist_ok=True)
    manifest = {
        "document_id": U(hash(name) & 0xffff | 0x10000),
        "engine_algorithm_set": "canopy-1",
        "files": {"graph": "graph.json", "materials": "materials.json",
                  "properties": "properties.json"},
        "format": "canopy-authoring", "handedness": "right", "name": name,
        "schema_version": "1.0.0", "seed": seed, "units": "meter", "up_axis": "Y",
    }
    nodes, props = [], {}
    for g in generators:
        nodes.append({"enabled": True, "id": g["id"], "name": g["name"],
                      "parent": g.get("parent"), "type": g["type"]})
        if g.get("props"):
            props[g["id"]] = g["props"]
    nodes.sort(key=lambda n: n["id"])
    files = {
        "manifest.json": manifest,
        "graph.json": {"nodes": nodes},
        "properties.json": {"generators": dict(sorted(props.items()))},
        "materials.json": {"materials": materials},
    }
    for fname, data in files.items():
        with open(os.path.join(d, fname), "w") as f:
            json.dump(data, f, indent=2, sort_keys=True)
            f.write("\n")
    print(f"wrote {d}")

BARK_COLOR = [0.42, 0.33, 0.24, 1.0]
def mat(mid, name, color, two_sided=False, cutout=None):
    m = {"id": mid, "name": name, "base_color": color, "two_sided": two_sided}
    if cutout:
        m["cutout"] = {"stem": [0, 0], "vertices": cutout}
    return m

def mirror_outline(right_side):
    """Build a closed symmetric outline from stem → right side → tip → left side."""
    out = [[0.0, 0.0]] + right_side + [[0.0, 1.0]]
    out += [[-x, y] for x, y in reversed(right_side)]
    return out

OAK_OUTLINE = mirror_outline([
    [0.07, 0.04], [0.17, 0.13], [0.11, 0.25], [0.25, 0.35], [0.15, 0.47],
    [0.28, 0.58], [0.16, 0.70], [0.20, 0.82], [0.08, 0.92],
])
WILLOW_OUTLINE = mirror_outline([
    [0.30, 0.2], [0.40, 0.45], [0.30, 0.7], [0.12, 0.9],
])
BARK = mat(U(0xB001), "bark_default", BARK_COLOR)
OAK_LEAF = mat(U(0xC001), "leaf_oak", [0.32, 0.45, 0.18, 1.0], True, OAK_OUTLINE)
FIR_NEEDLE = mat(U(0xC002), "needle_fir", [0.20, 0.34, 0.22, 1.0], True)
WILLOW_LEAF = mat(U(0xC003), "leaf_willow", [0.50, 0.58, 0.28, 1.0], True, WILLOW_OUTLINE)

def leaf_gen(gid, name, parent, mat, **p):
    props = {"generation.spacing.relative": p.get("spacing", 0.1),
             "generation.first": p.get("first", 0.15),
             "generation.last": p.get("last", 1.0),
             "generation.leaves_per_point": p.get("per_point", 2),
             "leaf.length.absolute": p.get("length", 0.08),
             "leaf.width.relative": p.get("width", 0.6),
             "leaf.size.variance.relative": p.get("size_var", 0.3),
             "leaf.pitch.degrees": p.get("pitch", 45),
             "leaf.pitch.variance.degrees": p.get("pitch_var", 20),
             "leaf.droop.relative": p.get("droop", 0.2),
             "leaf.fold.degrees": p.get("fold", 18),
             "material.leaf": mat["id"]}
    return {"id": gid, "name": name, "type": "canopy.batched_leaf",
            "parent": parent, "props": props}


# --- Coastal Oak: broad gnarled crown --------------------------------------
oak = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 5.0, "spine.radius.absolute": 0.38,
        "spine.bend.degrees": 14, "spine.wander.degrees": 7,
        "spine.flare.relative": 0.55, "spine.flare.length.relative": 0.18,
        "spine.flare.lobes": 6, "spine.flare.lobe.relative": 0.22,
        "mesh.uv.random_phase": True,
        "mesh.radial_segments": 18, "material.bark": BARK["id"],
        "spine.radius.profile": curve([[0, 1], [0.25, 0.8], [0.85, 0.38], [1, 0.0]]),
    }},
    {"id": U(3), "name": "Boughs", "type": "canopy.branch", "parent": U(2), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.075,
        "generation.first": 0.32, "generation.last": 0.95,
        "generation.angle.degrees": 55, "generation.angle.variance.degrees": 16,
        "spine.length.absolute": 3.3, "spine.length.variance.relative": 0.25,
        "spine.radius.absolute": 0.13, "spine.radius.variance.relative": 0.2,
        "spine.bend.degrees": 24, "spine.wander.degrees": 12,
        "spine.flare.relative": 0.35, "spine.flare.length.relative": 0.06,
        "mesh.uv.random_phase": True,
        "mesh.radial_segments": 10, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 1], [0.7, 0.88], [1, 0.5]]),
        "spine.radius.profile": curve([[0, 1], [1, 0.22]]),
    }},
    {"id": U(4), "name": "Branches", "type": "canopy.branch", "parent": U(3), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.16,
        "generation.first": 0.25, "generation.last": 0.95,
        "generation.angle.degrees": 48, "generation.angle.variance.degrees": 22,
        "spine.length.absolute": 1.5, "spine.length.variance.relative": 0.3,
        "spine.radius.absolute": 0.048, "spine.radius.variance.relative": 0.25,
        "spine.bend.degrees": 18, "spine.wander.degrees": 15,
        "mesh.radial_segments": 7, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 1], [1, 0.55]]),
        "spine.radius.profile": curve([[0, 1], [1, 0.2]]),
    }},
    {"id": U(5), "name": "Twigs", "type": "canopy.branch", "parent": U(4), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.22,
        "generation.first": 0.2, "generation.last": 1.0,
        "generation.angle.degrees": 55, "generation.angle.variance.degrees": 25,
        "spine.length.absolute": 0.55, "spine.length.variance.relative": 0.35,
        "spine.radius.absolute": 0.016, "spine.radius.variance.relative": 0.3,
        "spine.wander.degrees": 18,
        "mesh.radial_segments": 5, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 1], [1, 0.5]]),
    }},
]
oak.append({"id": U(8), "name": "Roots", "type": "canopy.branch", "parent": U(2), "props": {
    "generation.mode": "absolute", "generation.count": 6,
    "generation.first": 0.005, "generation.last": 0.02,
    "generation.angle.degrees": 112, "generation.angle.variance.degrees": 8,
    "spine.length.absolute": 1.7, "spine.length.variance.relative": 0.3,
    "spine.radius.absolute": 0.17, "spine.radius.variance.relative": 0.2,
    "spine.bend.degrees": 8, "spine.wander.degrees": 10,
    "spine.flare.relative": 0.5, "spine.flare.length.relative": 0.3,
    "spine.ground.level": 0.01,
    "mesh.radial_segments": 9, "material.bark": BARK["id"],
    "spine.radius.profile": curve([[0, 1], [1, 0.05]]),
}})
oak.append(leaf_gen(U(6), "Leaves", U(5), OAK_LEAF, spacing=0.12, first=0.2,
                    per_point=3, length=0.105, width=0.7, pitch=50, pitch_var=25,
                    droop=0.3, fold=26))
oak.append(leaf_gen(U(7), "InnerLeaves", U(4), OAK_LEAF, spacing=0.16, first=0.5,
                    per_point=2, length=0.095, width=0.7, pitch=55, pitch_var=25,
                    droop=0.3, fold=26))
write("CoastalOak", 421771, oak, [BARK, OAK_LEAF])

# --- Alpine Fir: conical conifer with upturned whorls -----------------------
fir = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 9.0, "spine.radius.absolute": 0.3,
        "spine.wander.degrees": 2,
        "spine.flare.relative": 0.4, "spine.flare.length.relative": 0.1,
        "spine.flare.lobes": 5, "spine.flare.lobe.relative": 0.15,
        "mesh.radial_segments": 14, "material.bark": BARK["id"],
        "spine.radius.profile": curve([[0, 1], [0.1, 0.82], [1, 0.0]]),
    }},
    {"id": U(3), "name": "Whorls", "type": "canopy.branch", "parent": U(2), "props": {
        "generation.mode": "phyllotaxy", "generation.internode.absolute": 0.55,
        "generation.members_per_whorl": 5, "generation.divergence.degrees": 137.5,
        "generation.azimuth.variance.degrees": 7,
        "generation.first": 0.09, "generation.last": 0.97,
        "generation.angle.degrees": 96, "generation.angle.variance.degrees": 6,
        "spine.length.absolute": 2.5, "spine.length.variance.relative": 0.12,
        "spine.radius.absolute": 0.05, "spine.radius.variance.relative": 0.15,
        "spine.bend.degrees": -14, "spine.wander.degrees": 4,
        "spine.flare.relative": 0.3, "spine.flare.length.relative": 0.08,
        "mesh.radial_segments": 7, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 1], [0.55, 0.58], [1, 0.05]]),
        "spine.radius.profile": curve([[0, 1], [1, 0.12]]),
    }},
    {"id": U(4), "name": "Sprays", "type": "canopy.branch", "parent": U(3), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.09,
        "generation.first": 0.12, "generation.last": 0.95,
        "generation.angle.degrees": 68, "generation.angle.variance.degrees": 14,
        "spine.length.absolute": 0.55, "spine.length.variance.relative": 0.3,
        "spine.radius.absolute": 0.013, "spine.radius.variance.relative": 0.25,
        "spine.bend.degrees": -8, "spine.wander.degrees": 8,
        "mesh.radial_segments": 5, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 1], [1, 0.4]]),
    }},
]
fir.append(leaf_gen(U(5), "Needles", U(4), FIR_NEEDLE, spacing=0.05, first=0.06,
                    per_point=5, length=0.05, width=0.22, pitch=55, pitch_var=18,
                    droop=0.08, fold=6, size_var=0.2))
fir.append(leaf_gen(U(6), "WhorlNeedles", U(3), FIR_NEEDLE, spacing=0.045, first=0.35,
                    per_point=4, length=0.045, width=0.22, pitch=60, pitch_var=18,
                    droop=0.08, fold=6, size_var=0.2))
write("AlpineFir", 90210, fir, [BARK, FIR_NEEDLE])

# --- Weeping Willow: heavy drooping streamers -------------------------------
willow = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 4.6, "spine.radius.absolute": 0.34,
        "spine.bend.degrees": 10, "spine.wander.degrees": 6,
        "spine.flare.relative": 0.6, "spine.flare.length.relative": 0.22,
        "spine.flare.lobes": 5, "spine.flare.lobe.relative": 0.18,
        "mesh.radial_segments": 16, "material.bark": BARK["id"],
        "spine.radius.profile": curve([[0, 1], [1, 0.4]]),
    }},
    {"id": U(3), "name": "Limbs", "type": "canopy.branch", "parent": U(2), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.08,
        "generation.first": 0.45, "generation.last": 0.98,
        "generation.angle.degrees": 38, "generation.angle.variance.degrees": 16,
        "spine.length.absolute": 2.7, "spine.length.variance.relative": 0.2,
        "spine.radius.absolute": 0.085, "spine.radius.variance.relative": 0.15,
        "spine.bend.degrees": 34, "spine.wander.degrees": 9,
        "spine.flare.relative": 0.35, "spine.flare.length.relative": 0.07,
        "mesh.radial_segments": 8, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 0.85], [0.8, 1.0], [1, 0.9]]),
        "spine.radius.profile": curve([[0, 1], [1, 0.3]]),
    }},
    {"id": U(4), "name": "Streamers", "type": "canopy.branch", "parent": U(3), "props": {
        "generation.mode": "interval", "generation.spacing.relative": 0.085,
        "generation.first": 0.25, "generation.last": 1.0,
        "generation.angle.degrees": 65, "generation.angle.variance.degrees": 18,
        "spine.length.absolute": 3.0, "spine.length.variance.relative": 0.25,
        "spine.radius.absolute": 0.02, "spine.radius.variance.relative": 0.2,
        "spine.bend.degrees": 150, "spine.wander.degrees": 6,
        "mesh.radial_segments": 5, "material.bark": BARK["id"],
        "child.length.profile": curve([[0, 0.75], [1, 1.05]]),
    }},
]
willow.append(leaf_gen(U(5), "Leaves", U(4), WILLOW_LEAF, spacing=0.035, first=0.08,
                       per_point=3, length=0.13, width=0.35, pitch=35, pitch_var=14,
                       droop=0.5, fold=14, size_var=0.25))
write("WeepingWillow", 7351, willow, [BARK, WILLOW_LEAF])


# --- Island Palm: frond generator showcase ----------------------------------
PALM_BARK = mat(U(0xB002), "bark_palm", [0.48, 0.40, 0.30, 1.0])
PALM_FROND = mat(U(0xC004), "frond_palm", [0.24, 0.46, 0.19, 1.0], True)

def frond_gen(gid, name, parent, count, angle, angle_var, bend, length):
    return {"id": gid, "name": name, "type": "canopy.frond", "parent": parent, "props": {
        "generation.mode": "absolute", "generation.count": count,
        "generation.first": 0.985, "generation.last": 1.0,
        "generation.angle.degrees": angle, "generation.angle.variance.degrees": angle_var,
        "spine.length.absolute": length, "spine.length.variance.relative": 0.15,
        "spine.bend.degrees": bend, "spine.wander.degrees": 4,
        "frond.width.absolute": 0.55, "frond.fold.degrees": 55,
        "frond.serration.count": 26, "frond.serration.depth": 0.7,
        "frond.twist.degrees": 15,
        "material.frond": PALM_FROND["id"],
    }}

palm = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 7.5, "spine.radius.absolute": 0.24,
        "spine.bend.degrees": 20, "spine.wander.degrees": 3,
        "spine.flare.relative": 0.7, "spine.flare.length.relative": 0.08,
        "mesh.uv.random_phase": True,
        "mesh.radial_segments": 12, "material.bark": PALM_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [0.08, 0.62], [1, 0.42]]),
    }},
    frond_gen(U(3), "CrownFronds", U(2), 8, 28, 16, 55, 2.9),
    frond_gen(U(4), "SkirtFronds", U(2), 11, 68, 18, 85, 2.6),
]
write("IslandPalm", 55155, palm, [PALM_BARK, PALM_FROND])


# --- River Birch: recursive bifurcation showcase ----------------------------
BIRCH_BARK = mat(U(0xB003), "bark_birch", [0.72, 0.68, 0.60, 1.0])
BIRCH_LEAF = mat(U(0xC005), "leaf_birch", [0.42, 0.55, 0.22, 1.0], True, mirror_outline([
    [0.28, 0.18], [0.38, 0.42], [0.26, 0.68], [0.10, 0.88],
]))

def fork_gen(gid, name, parent, phase, angle, length, radius, taper_end, wander, count=2):
    return {"id": gid, "name": name, "type": "canopy.branch", "parent": parent, "props": {
        "generation.mode": "bifurcation", "generation.count": count,
        "generation.phase.degrees": phase,
        "generation.azimuth.variance.degrees": 20,
        "generation.angle.degrees": angle, "generation.angle.variance.degrees": 9,
        "spine.length.absolute": length, "spine.length.variance.relative": 0.18,
        "spine.radius.absolute": radius, "spine.radius.variance.relative": 0.12,
        "spine.bend.degrees": 10, "spine.wander.degrees": wander,
        "spine.flare.relative": 0.25, "spine.flare.length.relative": 0.06,
        "mesh.radial_segments": 8, "material.bark": BIRCH_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [1, taper_end]]),
    }}

birch = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 3.8, "spine.radius.absolute": 0.2,
        "spine.bend.degrees": 6, "spine.wander.degrees": 5,
        "spine.flare.relative": 0.45, "spine.flare.length.relative": 0.14,
        "spine.flare.lobes": 4, "spine.flare.lobe.relative": 0.12,
        "mesh.uv.random_phase": True,
        "mesh.radial_segments": 14, "material.bark": BIRCH_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [1, 0.62]]),
    }},
    fork_gen(U(3), "Fork1", U(2), 0, 26, 2.3, 0.125, 0.62, 7),
    fork_gen(U(4), "Fork2", U(3), 90, 30, 1.7, 0.075, 0.55, 9, count=3),
    fork_gen(U(5), "Fork3", U(4), 45, 33, 1.2, 0.042, 0.35, 12, count=3),
    {"id": U(6), "name": "Twigs", "type": "canopy.branch", "parent": U(5), "props": {
        "generation.mode": "proportional", "generation.density.per_meter": 9,
        "generation.first": 0.15, "generation.last": 1.0,
        "generation.angle.degrees": 55, "generation.angle.variance.degrees": 22,
        "spine.length.absolute": 0.45, "spine.length.variance.relative": 0.35,
        "spine.radius.absolute": 0.011, "spine.radius.variance.relative": 0.3,
        "spine.wander.degrees": 16,
        "mesh.radial_segments": 5, "material.bark": BIRCH_BARK["id"],
        "child.length.profile": curve([[0, 1], [1, 0.6]]),
    }},
]
birch.append(leaf_gen(U(7), "Leaves", U(6), BIRCH_LEAF, spacing=0.15, first=0.15,
                      per_point=3, length=0.08, width=0.75, pitch=50, pitch_var=25,
                      droop=0.25, fold=20))
birch.append(leaf_gen(U(8), "InnerLeaves", U(5), BIRCH_LEAF, spacing=0.14, first=0.45,
                      per_point=2, length=0.07, width=0.75, pitch=55, pitch_var=25,
                      droop=0.25, fold=20))
write("RiverBirch", 24601, birch, [BIRCH_BARK, BIRCH_LEAF])


# --- Nebula Spore Tree: alien flora stress test -----------------------------
# Same generators, hostile art direction: candelabra bifurcation, corkscrew
# tentacle fronds, luminous spore-pod cutouts.
XENO_BARK = mat(U(0xB004), "bark_xeno", [0.24, 0.17, 0.32, 1.0])
XENO_TENDRIL = mat(U(0xC006), "tendril_xeno", [0.10, 0.72, 0.62, 1.0], True)
XENO_POD = mat(U(0xC007), "pod_xeno", [0.82, 0.26, 0.74, 1.0], True, mirror_outline([
    [0.30, 0.12], [0.46, 0.38], [0.44, 0.66], [0.24, 0.90],
]))

xeno = [
    {"id": U(1), "name": "Tree", "type": "canopy.tree"},
    {"id": U(2), "name": "Trunk", "type": "canopy.branch", "parent": U(1), "props": {
        "generation.mode": "absolute", "generation.count": 1,
        "spine.length.absolute": 5.2, "spine.radius.absolute": 0.34,
        "spine.bend.degrees": 9, "spine.wander.degrees": 14,
        "spine.flare.relative": 1.1, "spine.flare.length.relative": 0.28,
        "spine.flare.lobes": 8, "spine.flare.lobe.relative": 0.38,
        "mesh.uv.random_phase": True,
        "mesh.radial_segments": 18, "material.bark": XENO_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [0.3, 0.72], [1, 0.34]]),
    }},
    {"id": U(3), "name": "Arms", "type": "canopy.branch", "parent": U(2), "props": {
        "generation.mode": "bifurcation", "generation.count": 5,
        "generation.phase.degrees": 12, "generation.azimuth.variance.degrees": 10,
        "generation.angle.degrees": 68, "generation.angle.variance.degrees": 8,
        "spine.length.absolute": 2.8, "spine.length.variance.relative": 0.18,
        "spine.radius.absolute": 0.10, "spine.radius.variance.relative": 0.15,
        "spine.bend.degrees": -42, "spine.wander.degrees": 12,
        "spine.flare.relative": 0.35, "spine.flare.length.relative": 0.08,
        "mesh.radial_segments": 9, "material.bark": XENO_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [1, 0.3]]),
    }},
    {"id": U(4), "name": "Talons", "type": "canopy.branch", "parent": U(3), "props": {
        "generation.mode": "bifurcation", "generation.count": 3,
        "generation.phase.degrees": 60, "generation.azimuth.variance.degrees": 15,
        "generation.angle.degrees": 44, "generation.angle.variance.degrees": 10,
        "spine.length.absolute": 1.5, "spine.length.variance.relative": 0.22,
        "spine.radius.absolute": 0.048, "spine.radius.variance.relative": 0.2,
        "spine.bend.degrees": -34, "spine.wander.degrees": 15,
        "mesh.radial_segments": 7, "material.bark": XENO_BARK["id"],
        "spine.radius.profile": curve([[0, 1], [1, 0.2]]),
    }},
    {"id": U(5), "name": "Tendrils", "type": "canopy.frond", "parent": U(4), "props": {
        "generation.mode": "absolute", "generation.count": 4,
        "generation.first": 0.88, "generation.last": 1.0,
        "generation.angle.degrees": 42, "generation.angle.variance.degrees": 30,
        "spine.length.absolute": 1.35, "spine.length.variance.relative": 0.25,
        "spine.bend.degrees": 58, "spine.wander.degrees": 9,
        "frond.width.absolute": 0.17, "frond.fold.degrees": 14,
        "frond.serration.count": 18, "frond.serration.depth": 0.55,
        "frond.twist.degrees": 540,
        "material.frond": XENO_TENDRIL["id"],
    }},
]
xeno.append(leaf_gen(U(6), "SporePods", U(4), XENO_POD, spacing=0.11, first=0.3,
                     per_point=2, length=0.17, width=1.05, pitch=78, pitch_var=18,
                     droop=0.08, fold=10, size_var=0.35))
write("NebulaSporeTree", 40424242, xeno, [XENO_BARK, XENO_TENDRIL, XENO_POD])

#!/usr/bin/env python3
"""Animated timeline showcases (ADR-0006): wind loop, growth sequence, and a
seasons strip, written into samples/renders/.

Each frame is a full deterministic CLI export at a timeline sample, rendered
by the diagnostic rasterizer — the "baked per-frame meshes for diagnostic
use" path from 13_WIND_GROWTH_AND_SEASONS.md. Requires numpy and Pillow.
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render  # noqa: E402

from PIL import Image  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CLI = os.path.join(REPO, 'build', 'headless-dev', 'apps', 'cli', 'canopy-cli')
PRESET = os.path.join(REPO, 'tests', 'presets', 'obj-diagnostic.json')
OUT = os.path.join(REPO, 'samples', 'renders')


def export_frame(document, out_base, **timeline):
    args = [CLI, 'export', os.path.join(REPO, 'samples', 'documents', document),
            '--preset', PRESET, '--out', out_base]
    flags = {'time': '--time', 'growth': '--growth', 'season': '--season',
             'wind_strength': '--wind-strength', 'wind_direction': '--wind-direction',
             'gust': '--gust'}
    for key, flag in flags.items():
        if key in timeline:
            args += [flag, str(timeline[key])]
    subprocess.run(args, check=True, capture_output=True)


def frame_image(obj_base, png_path, **render_args):
    render([load_obj(obj_base + '.obj')], png_path, **render_args)
    return Image.open(png_path).convert('RGB')


def bbox_focus(obj_base):
    verts = load_obj(obj_base + ".obj")[0]
    lo, hi = verts.min(0), verts.max(0)
    center = tuple((lo + hi) / 2)
    size = float((hi - lo).max())
    return center, size


def save_gif(frames, path, duration_ms):
    frames[0].save(path, save_all=True, append_images=frames[1:],
                   duration=duration_ms, loop=0, optimize=True)
    print(f'wrote {path} ({os.path.getsize(path) // 1024} KB, {len(frames)} frames)')


def wind_gif(tmp):
    """Coastal Oak in a gusty breeze; ping-pong loop for seamless playback.

    The camera is LOCKED to the calm pose's bounds: auto-framing each frame
    would follow the swaying crown's bounding box and make the whole image
    (base included) appear to drift.
    """
    calm = os.path.join(tmp, 'wind_calm')
    export_frame('CoastalOak.canopyproj', calm, wind_strength=0)
    center, size = bbox_focus(calm)
    frames = []
    count = 14
    for i in range(count):
        t = i * 0.42
        base = os.path.join(tmp, f'wind_{i}')
        export_frame('CoastalOak.canopyproj', base,
                     time=t, wind_strength=0.75, wind_direction=25, gust=0.6)
        frames.append(frame_image(base, base + '.png', w=560, h=700, azim_deg=30,
                                  focus_center=center, focus_size=size * 1.06,
                                  bases=[(0, 0, 0)]))
    path = os.path.join(OUT, 'oak_wind.gif')
    save_gif(frames + frames[-2:0:-1], path, 110)
    _assert_base_planted(path)


def _assert_base_planted(gif_path):
    """Guard: the ground/roots/trunk-foot band (bottom 15%) must be pixel-
    static across every wind frame — catches both geometry regressions
    (rigid sway) and camera regressions (per-frame auto-framing drift)."""
    import numpy as np
    from PIL import ImageSequence
    gif = Image.open(gif_path)
    seq = [np.asarray(f.convert('RGB'), dtype=np.int16)
           for f in ImageSequence.Iterator(gif)]
    h = seq[0].shape[0]
    ground = slice(int(h * 0.85), h)
    for frame in seq[1:]:
        changed = int((np.abs(frame - seq[0]).max(axis=2)[ground] > 12).sum())
        assert changed == 0, f'base region moved: {changed} px changed'
    print('base-planted check: OK (bottom 15% pixel-static across '
          f'{len(seq)} frames)')


def growth_gif(tmp):
    """River Birch lifecycle 0 → 1 with a camera fixed on the adult tree."""
    grown = os.path.join(tmp, 'growth_final')
    export_frame('RiverBirch.canopyproj', grown, growth=1.0)
    center, size = bbox_focus(grown)
    frames = []
    count = 16
    for i in range(count):
        # growth 0 is an empty model (nothing has emerged); start just after
        # germination so every frame has geometry.
        g = 0.03 + (1.0 - 0.03) * i / (count - 1)
        base = os.path.join(tmp, f'growth_{i}')
        export_frame('RiverBirch.canopyproj', base, growth=g)
        frames.append(frame_image(base, base + '.png', w=560, h=700, azim_deg=32,
                                  focus_center=center, focus_size=size,
                                  bases=[(0, 0, 0)]))
    # Hold the adult tree before looping back to the seed.
    save_gif(frames + [frames[-1]] * 5, os.path.join(OUT, 'birch_growth.gif'), 170)


def season_strip(tmp):
    """Coastal Oak across the season track: summer → autumn color → drop."""
    # Fixed camera across panels (leaf drop shrinks the bounding box).
    summer = os.path.join(tmp, 'season_ref')
    export_frame('CoastalOak.canopyproj', summer, season=0.45)
    center, size = bbox_focus(summer)
    panels = []
    for season in (0.45, 0.72, 0.85, 0.97):
        base = os.path.join(tmp, f'season_{int(season * 100)}')
        export_frame('CoastalOak.canopyproj', base, season=season)
        panels.append(frame_image(base, base + '.png', w=480, h=640, azim_deg=30,
                                  focus_center=center, focus_size=size,
                                  bases=[(0, 0, 0)]))
    strip = Image.new('RGB', (sum(p.width for p in panels), panels[0].height))
    x = 0
    for panel in panels:
        strip.paste(panel, (x, 0))
        x += panel.width
    path = os.path.join(OUT, 'oak_seasons.png')
    strip.save(path)
    print(f'wrote {path} ({os.path.getsize(path) // 1024} KB)')


if __name__ == '__main__':
    with tempfile.TemporaryDirectory(prefix='canopy-anim-') as tmp:
        wind_gif(tmp)
        growth_gif(tmp)
        season_strip(tmp)

#!/usr/bin/env python3
"""LOD profile comparison strip: the same document evaluated at draft,
preview, and production density (14_LOD groundwork; profiles change sample
and segment counts, never semantic identity). Writes samples/renders/.
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render  # noqa: E402

from PIL import Image, ImageDraw  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CLI = os.path.join(REPO, 'build', 'headless-dev', 'apps', 'cli', 'canopy-cli')
OUT = os.path.join(REPO, 'samples', 'renders')


def lod_strip():
    panels = []
    with tempfile.TemporaryDirectory(prefix='canopy-lod-') as tmp:
        preset = os.path.join(tmp, 'preset.json')
        for profile in ('draft', 'preview', 'production'):
            with open(preset, 'w') as f:
                f.write('{"format": "obj", "profile": "%s"}\n' % profile)
            base = os.path.join(tmp, profile)
            result = subprocess.run(
                [CLI, 'export', os.path.join(REPO, 'samples', 'documents',
                                             'CoastalOak.canopyproj'),
                 '--preset', preset, '--out', base, '--json'],
                check=True, capture_output=True, text=True)
            import json as jsonlib
            tris = jsonlib.loads(result.stdout)['triangle_count']
            png = base + '.png'
            render([load_obj(base + '.obj')], png, w=480, h=640, azim_deg=30)
            panel = Image.open(png).convert('RGB')
            draw = ImageDraw.Draw(panel)
            draw.text((16, 12), f'{profile}: {tris:,} tris', fill=(60, 56, 48))
            panels.append(panel)
    strip = Image.new('RGB', (sum(p.width for p in panels), panels[0].height))
    x = 0
    for panel in panels:
        strip.paste(panel, (x, 0))
        x += panel.width
    path = os.path.join(OUT, 'lod_profiles.png')
    strip.save(path)
    print(f'wrote {path} ({os.path.getsize(path) // 1024} KB)')


if __name__ == '__main__':
    lod_strip()

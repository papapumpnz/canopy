#!/usr/bin/env python3
"""Runtime forest showcase: compile species to .canopyrt, bake a deterministic
CPU forest through canopy-runtime-demo (runtime libraries only), and render
the visible set into samples/renders/forest.png.
"""
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render_showcase import load_obj, render  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CLI = os.path.join(REPO, 'build', 'headless-dev', 'apps', 'cli', 'canopy-cli')
DEMO = os.path.join(REPO, 'build', 'headless-dev', 'apps', 'runtime-viewer',
                    'canopy-runtime-demo')
RT = os.path.join(REPO, 'build', 'rt')
OUT = os.path.join(REPO, 'samples', 'renders')

SPECIES = ['CoastalOak', 'RiverBirch', 'WeepingWillow', 'IslandPalm']


def main():
    models = []
    for name in SPECIES:
        base = os.path.join(RT, name)
        subprocess.run([CLI, 'compile',
                        os.path.join(REPO, 'samples', 'documents', f'{name}.canopyproj'),
                        '--out', base], check=True, capture_output=True)
        models.append(base + '.canopyrt')
    forest_base = os.path.join(RT, 'forest')
    result = subprocess.run(
        [DEMO, *models, '--out', forest_base, '--seed', '20', '--count', '42',
         '--extent', '32', '--spacing', '4.2', '--camera', '0', '4', '70'],
        check=True, capture_output=True, text=True)
    stats = json.loads(result.stdout)
    print('forest stats:', stats)

    corners = [(-34, 0, -34), (34, 0, -34), (-34, 0, 34), (34, 0, 34), (0, 0, 0)]
    render([load_obj(forest_base + '.obj')], os.path.join(OUT, 'forest.png'),
           w=1700, h=950, azim_deg=3, fov=30, bases=corners,
           focus_center=(0, 4.5, 0), focus_size=34)


if __name__ == '__main__':
    main()

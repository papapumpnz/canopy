# Verdant and Ash vegetation concepts

Six standalone Canopy-native concept assets: major tree, minor tree and shrub
for each biome. They are deliberately not registered with World Studio.

The crowns use alpha-masked compound foliage clusters on two-triangle cards.
Layered sun/shade card materials provide canopy depth while keeping geometry
well below per-leaf-mesh costs. Canopy's draft/preview/production evaluation
profiles and runtime impostors remain available for lower LODs.

Regenerate from the repository root:

```text
python3 tools/showcase/generate_biome_concepts.py
cmake --build --preset headless-dev
for project in samples/concepts/documents/*.canopyproj; do
  name="$(basename "$project" .canopyproj)"
  build/headless-dev/apps/cli/canopy-cli export "$project" \
    --preset tests/presets/obj-diagnostic.json \
    --out "build/biome-concepts/$name"
done
python3 tools/showcase/render_biome_concepts.py
```

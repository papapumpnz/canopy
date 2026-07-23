# Cartoon canopy concepts

Seven standalone Canopy-native concept trees in one stylised cartoon family:
three large canopied trees, two medium and two small. Like the biome set in
`samples/concepts/`, these are art-direction candidates and are deliberately
not registered with World Studio.

| Document | Class | Draft tris | Silhouette |
|---|---|---:|---|
| `ToonBigTop` | large | 3,192 | fat trunk under one enormous rounded canopy |
| `ToonBroadSpread` | large | 1,427 | squat trunk, twin fork, broad flattened crown |
| `ToonTallStack` | large | 1,830 | tall storybook leader with stacked cloud tiers |
| `ToonRoundTop` | medium | 1,958 | stout everyday filler with a round crown |
| `ToonLeaner` | medium | 1,128 | character tree with a strong lean and off-centre crown |
| `ToonSapling` | small | 261 | young single stem and a small puff |
| `ToonBush` | small | 1,194 | round multi-stem undergrowth dressing |

## How the look is built

Three choices carry the style, and all three also keep the triangle count down:

* **Chunky proportions.** Short, very thick trunks with a heavy smooth flare
  and a slow taper. Flare *lobes* are unused throughout — they alias badly
  against a 5–6 segment ring, so the buttress character comes from
  `spine.flare.relative` plus a few explicit fat root branches.
* **Few, large foliage cards.** Every foliage material sets `card_region`,
  which is the evaluator's two-triangles-per-leaf path, and each card shows a
  whole hand-painted leaf blob with a hard toon rim. A full crown costs a few
  hundred triangles rather than tens of thousands of per-leaf quads. Note that
  `leaf.fold.degrees` is ignored on this path, so the documents do not set it.
* **Low radial segments, authored in the document.** Trunks 6, boughs 5, twigs
  4. Because these live in the document rather than relying on the export
  profile's cap, the trees read as low-poly at every profile; the draft profile
  only thins the ring count further along each spine.

Seeds are picked from a per-species sweep rather than left arbitrary. Bough
azimuth is a random draw — the evaluator exposes no azimuth spread control
outside phyllotaxy and bifurcation — so with only 4–8 boughs the seed is what
decides whether a crown reads round or lopsided.

Exported with the draft profile (`tools/showcase/presets/obj-cartoon.json`)
these run 261–3,192 triangles; at the production profile the same documents
run 499–7,850.

## Renders

`renders/` holds a hero still and an 18-frame turntable GIF per species, two
contact sheets, and `cartoon_lineup.png` — all seven at a shared scale so the
large/medium/small relationship is legible.

Both are produced through the rasterizer's alpha path. Its opaque path samples
a single texel at each triangle's UV centroid, which collapses every foliage
card into a solid triangle and shreds the silhouette.

Everything here is palette-indexed — 64 colours for the GIFs, 256 for the PNGs
— which is visually lossless on flat-shaded art and roughly halves the
directory. 64 is the floor: below it the trunk browns drift olive. Dithering
is off, since it would scatter otherwise-identical pixels between frames and
defeat Pillow's inter-frame optimisation.

## Regenerate

From the repository root:

```text
python3 tools/showcase/generate_cartoon_concepts.py
cmake --build --preset headless-dev
for project in samples/cartoon/documents/*.canopyproj; do
  name="$(basename "$project" .canopyproj)"
  build/headless-dev/apps/cli/canopy-cli export "$project" \
    --preset tools/showcase/presets/obj-cartoon.json \
    --out "build/cartoon/$name"
done
python3 tools/showcase/render_cartoon_concepts.py
```

The generator is deterministic: textures and documents are synthesized from
fixed seeds, so two consecutive runs produce byte-identical output.

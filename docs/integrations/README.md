# Integrating Canopy output into other products

Canopy is headless-first: every artifact a host product needs is produced by
`canopy-cli` from a `.canopyproj` document, deterministically. Integrations
consume **exported artifacts**, never the authoring document, so your pipeline
stays decoupled from Canopy internals.

Current bootstrap surface (grows with `16_EXPORT_PIPELINE.md` /
`18_ENGINE_AND_DCC_INTEGRATIONS.md`):

| Artifact | Contents |
|---|---|
| `<name>.obj` | Triangulated geometry, one `g`/`usemtl` group per branch node (`sem_<semantic-id>`) |
| `<name>.mtl` | Referenced material stubs |
| `<name>.manifest.json` | Counts, SHA-256 of the OBJ, model/topology hashes, profile |

glTF export is the next planned exporter and will become the recommended
runtime interchange for web engines; the manifest contract stays the same.

## The pipeline contract

```text
canopy-cli export Tree.canopyproj --preset presets/obj-diagnostic.json --out dist/tree --json
```

- The command is **deterministic**: same document + preset + Canopy version →
  byte-identical OBJ and identical `model_hash`. Cache artifacts by
  `document_hash` + preset; rebuild only when either changes.
- `--json` prints a machine-readable result envelope (`ok`, hashes, counts) so
  build systems can gate on it without parsing logs.
- Always verify `obj_sha256` from the manifest after transfer; it is the
  integrity contract between your asset pipeline and runtime bundle.

Guides:

- [`three_js.md`](three_js.md) — loading Canopy exports in three.js
- More hosts (Unity, Unreal, Godot, USD/DCC) arrive with their exporters per
  the design pack; the manifest-driven pattern shown for three.js applies to
  any engine that can load OBJ/glTF.

# Asset library and variation system

## Scope

Canopy Library organizes models, materials, textures, meshes, templates, presets and generated variants. It supports an entirely local library and an optional self-hosted or commercial service. The authoring and runtime products remain functional without a network connection.

The project does not include third-party proprietary vegetation content. All example assets must be original, commissioned under compatible terms, generated from public-domain measurements, or drawn from clearly compatible repositories with recorded provenance.

## Asset package

Library assets use `.canopyasset`, a ZIP container with a canonical manifest and content-addressed payloads.

```yaml
schema: canopy.asset/1
id: org.example.species.quercus-robur.mature-01
version: 1.2.0
kind: vegetation-model
name: English oak, mature 01
species:
  scientific_name: Quercus robur
  common_names: [English oak, pedunculate oak]
traits:
  biome: [temperate-deciduous]
  form: [broadleaf-tree]
  height_m: [18, 30]
license:
  spdx: CC-BY-4.0
  attribution: Example Studio
  source_url: null
entrypoints:
  authoring: model/Oak.canopyproj
  runtime: runtime/Oak.canopyrt
previews:
  thumbnail: previews/thumb.webp
  turntable: previews/turntable.mp4
variants:
  - presets/young.yaml
  - presets/mature.yaml
```

The manifest also stores checksums, dependencies, minimum feature versions, geographic metadata, tags, quality tier and known restrictions.

## Catalog model

Catalog records include:

- Stable package ID and semantic version
- Human-readable name and description
- Scientific taxonomy where applicable
- Common names and locale variants
- Plant form, biome, climate, hardiness and geographic region
- Age, season, health and style categories
- Polygon, material, texture and memory statistics
- Available authoring, game, VFX and runtime representations
- Required plugins
- License and provenance
- Review and validation status
- Preview media hashes

Search uses exact fields, full text, tags and numeric ranges. Scientific names are stored separately from display translations.

## Local library

The local service provides:

- Indexed folders and package stores
- Watch-based incremental updates
- Dependency resolution
- Offline preview cache
- Duplicate detection by content hash
- Package validation and quarantine
- User collections, favorites and recent items
- Source-control-friendly project references
- Configurable shared-team paths

The index is disposable and can be rebuilt from manifests. SQLite is suitable for the local catalog; packages remain ordinary files.

## Remote service

An optional remote service exposes:

- Catalog search
- Package and delta download
- Version and dependency metadata
- Entitlement checks for private packages
- Organization collections
- Review and approval workflow
- Audit records
- Signed package manifests
- Content delivery through standard object storage

The protocol is documented and server implementations are replaceable. Credentials live in operating-system secure storage. The Modeler communicates only when the user configures a service.

## Dependency resolution

Dependencies use package IDs and semantic version ranges. Resolution is lockfile based.

`canopy.lock` records:

- Exact package versions
- Content hashes
- Registry origin
- License identifier
- Plugin requirements
- Transitive dependency graph

Opening a project with missing dependencies offers resolution but never substitutes a newer package silently. A vendoring command can copy dependencies into the project.

## Variation model

A base document can expose a `VariationSchema` containing named controls and constraints. A variant stores only seed and overrides.

Variation dimensions include:

- Global random seed
- Generator-specific seeds
- Age and growth phase
- Season and phenological state
- Health, damage and pruning state
- Crown asymmetry
- Trunk lean, height and taper
- Branch density and internode spacing
- Leaf size, density, hue and occupancy
- Wind profile
- Material set
- Target LOD and export profile

Constraints prevent invalid combinations, such as mature trunk dimensions with a seedling branch hierarchy unless intentionally allowed.

## Deterministic variant generation

A `VariantRecipe` defines distributions and correlations:

```yaml
schema: canopy.variant-recipe/1
id: oak-production-set
count: 24
seed: 9001
parameters:
  height_scale:
    distribution: truncated_normal
    mean: 1.0
    standard_deviation: 0.12
    minimum: 0.72
    maximum: 1.28
  crown_density:
    distribution: uniform
    minimum: 0.75
    maximum: 1.25
correlations:
  - variables: [height_scale, trunk_radius_scale]
    coefficient: 0.72
reject_when:
  - expression: estimated_triangles_lod0 > 150000
```

Generation uses named deterministic streams. Reordering unrelated parameters does not alter existing sampled values.

## Variation browser

The Modeler provides a grid of asynchronously evaluated thumbnails. Users can:

- Lock selected dimensions
- Reroll unlocked dimensions
- Compare geometry and material statistics
- Promote a variation to a new document
- Save a recipe or override set
- Export selected variants in batch
- Detect near-duplicates using parameter and silhouette distance

Thumbnail evaluation uses a bounded preview profile and cancelable tasks.

## Asset adaptation

Library assets remain editable within their license terms. The adaptation workflow supports:

- Global shape and density controls
- Season selection
- Randomization
- Material replacement
- Wind retuning
- LOD target selection
- Unit conversion
- Engine-specific export preset

Any locked or read-only package is imported as a reference. User edits are stored as a non-destructive overlay where possible.

## Provenance

Every payload records:

- Creator or organization
- Creation method
- Source files and scans where distributable
- License
- Attribution text
- Consent or property-release records where relevant
- Transformations and derived assets
- Tool versions
- Review history

Generated outputs carry a provenance manifest linking back to the source package and project revision.

## License policy engine

The library can evaluate license compatibility for a target use. Policy rules are data, not hard-coded legal conclusions.

Examples of machine-checkable restrictions:

- Attribution required
- Noncommercial use only
- Share-alike
- No redistribution of source package
- Editorial use
- Territory or expiration constraints
- Internal organization only

The UI presents exact recorded terms and blocks a package operation only when organization policy explicitly requires it.

## Content-quality tiers

Suggested tiers:

| Tier | Required representations |
|---|---|
| Draft | Authoring file and thumbnail |
| Standard | Validated model, PBR textures, at least three LODs, wind and collision |
| Production | Standard plus impostor, seasonal states, growth or age variants, engine presets and performance budgets |
| Hero VFX | High-resolution geometry, UDIM or high-resolution textures, animation caches and DCC validation |

Quality tier is a claim backed by validation results, not a marketing label.

## Package security

- All paths are normalized and confined to the extraction root.
- Checksums are verified before use.
- Compressed and uncompressed size limits are enforced.
- Script execution is not implicit.
- Native plugins cannot be bundled as ordinary content dependencies.
- Remote package signatures can be required by policy.
- Preview media is decoded in a constrained process where practical.

## Library acceptance criteria

- A user can search, install, update, pin, vendor and remove a package offline or through a configured registry.
- Project lockfiles reproduce exact dependency content.
- Variant recipes produce identical overrides across supported platforms.
- License and provenance fields survive package, project and export workflows.
- A corrupt or hostile archive cannot write outside the cache or exhaust configured resources.
- The product remains fully usable when all network services are unavailable.

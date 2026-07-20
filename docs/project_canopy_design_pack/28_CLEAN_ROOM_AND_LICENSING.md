# Clean-room implementation and licensing

## Objective

Project Canopy targets equivalent workflows and outputs without copying proprietary source code, private documentation, binary formats, shaders, assets, schemas, algorithms, UI artwork or distinctive expression. This document is an engineering control specification, not a substitute for jurisdiction-specific legal review.

## Permitted requirements inputs

The requirements team may use:

- Public product pages
- Public user manuals and release notes
- Publicly observable workflows using legitimately licensed software
- Published standards and official SDK documentation
- Academic literature
- General botanical, geometric, graphics and simulation knowledge
- Independently created measurements, examples and assets
- User interviews framed around needs and outcomes

Inputs are recorded by URL or publication, access date, scope extracted and contributor identity.

## Prohibited implementation inputs

Implementation contributors and agents must not use:

- Leaked, decompiled or disassembled binaries
- Reverse-engineered proprietary file layouts
- Private SDK headers or source obtained outside an authorized adapter team
- Proprietary shaders or copied code fragments
- Commercial library assets as test fixtures unless explicit redistribution and use terms permit it
- Screenshots, icons or layouts copied as UI source material beyond high-level workflow observation
- Confidential information from former or current employees
- Model outputs used to infer secret formats or algorithms where license terms prohibit that activity

When a contributor has prior exposure to protected implementation material, project counsel or the clean-room owner determines an appropriate separation or recusal.

## Team separation

For sensitive compatibility work, use two roles:

### Requirements room

- Observes public behavior and documents black-box requirements.
- Produces capability descriptions, inputs, expected outputs and tolerances.
- Does not propose internal implementation based on protected details.

### Implementation room

- Receives only approved clean-room requirements.
- Designs algorithms independently from public standards, literature and general expertise.
- Records architecture choices and sources.

The normal product can use one team for public feature analysis when contamination risk is low, but provenance rules still apply.

## Proprietary file formats

Native SpeedTree formats are explicitly outside the core scope. Project Canopy must not claim support for `.spm`, `.st`, `.st9`, `.stsdk`, `.ste` or other proprietary containers unless all of the following exist:

- Written authorization or license
- Separate adapter architecture
- Isolated licensed source/build environment
- Distribution-rights review
- Compatibility test assets with lawful provenance
- Public messaging approved for exact supported versions and limitations

The core product uses `.canopyproj`, `.canopy`, `.canopyasset`, `.canopyrt` and industry-standard interchange.

## Trademarks and presentation

- “Project Canopy” is a codename pending clearance.
- Third-party product names appear only to describe compatibility targets or factual comparisons.
- No logo, icon set, product color system or distinctive UI artwork is copied.
- Public claims use “functional alternative,” “workflow parity” or a precise capability statement.
- Avoid “drop-in replacement” unless binary and behavioral substitution is actually licensed, implemented and evidenced.
- Include trademark attribution where appropriate.

## Patent review

Procedural vegetation, impostor rendering, wind, LOD and related fields may contain active or historical patents. Before commercialization:

- Run a targeted freedom-to-operate review in intended markets.
- Record patents considered and claim scope relevant to implemented designs.
- Prefer independently published, expired, licensed or clearly non-infringing approaches.
- Preserve algorithm design notes showing independent derivation.
- Route newly identified patent notices through the release risk process.

Engineers do not make unrecorded legal conclusions from a patent abstract.

## Third-party software

Every dependency record includes:

- Project and canonical source
- Exact version and hash
- License and notice text
- Static/dynamic/header-only use
- Modifications
- Distribution obligations
- Patent clauses where present
- Platforms and optionality
- Security owner
- Upgrade and replacement path

Copyleft dependencies are allowed only when their obligations match the distribution model and receive explicit review. Optional proprietary SDKs remain isolated from open/core builds.

## Candidate dependency policy

The proposed stack includes permissively licensed or dual-licensed components, but each exact version must be reviewed at intake. Examples include Qt under an appropriate commercial or open-source compliance strategy, WebGPU implementation libraries, OpenUSD, Alembic, Lua, CMake, testing frameworks and image/mesh libraries.

No design-pack mention constitutes license approval.

## Asset and scan provenance

Every distributed example, texture, mesh, photograph and scan requires:

- Creator or source
- Creation date where known
- License or assignment
- Attribution text
- Model/property release where relevant
- Source URL or internal record
- Derivation history
- Restrictions on redistribution, training or commercial use

Synthetic geometry and procedural textures should be preferred for parser and algorithm tests. Real scans require an auditable capture or license record.

## AI-assisted contribution provenance

Each agent work item records:

- Model/tool identifier and execution date
- Human task author and reviewer
- Inputs supplied to the agent
- Repository revision
- Whether external retrieval was enabled
- Generated files and significant suggested dependencies
- Human verification performed

Agents must not be instructed to imitate proprietary source, reproduce decompiled logic, or translate protected code into another language.

## Source headers and notices

Original source files use the project copyright and license header policy. Third-party copied or adapted code retains required notices and is stored in clearly identified directories. Small standard-derived constants or test vectors cite their normative source.

## Contributor declaration

Each contributor affirms:

- The contribution is original or appropriately licensed.
- Protected implementation material was not used improperly.
- Third-party code/assets are identified.
- The contributor has authority to submit the work.
- Known obligations or conflicts are disclosed.

## Audit artifacts

A release audit includes:

- Contributor declarations
- Agent provenance logs
- Dependency lock and SBOM
- Third-party notices
- Asset provenance manifests
- ADR and algorithm-source index
- Trademark/name review
- Optional adapter licenses
- Feature-claim evidence
- Open legal or licensing risks

## Incident response

When contamination or licensing uncertainty is reported:

1. Quarantine affected files and derived artifacts.
2. Preserve evidence and identify commits/builds containing them.
3. Stop distribution of affected artifacts when required.
4. Have an independent reviewer assess the issue.
5. Replace with a clean implementation or remove the feature.
6. Regenerate golden files and packages from clean sources.
7. Document remediation and release impact.

Do not attempt to obscure provenance through rewrites.

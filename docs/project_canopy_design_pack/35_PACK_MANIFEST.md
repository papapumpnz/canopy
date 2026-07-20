# Pack manifest

## Baseline

- Pack: Project Canopy implementation design
- Baseline date: 2026-07-19
- Functional comparison point: publicly documented SpeedTree Modeler 10.2.0 and SpeedTree Runtime SDK 10
- Scope boundary: clean-room workflow and functional parity; no proprietary binary-format compatibility without a separately licensed adapter

## Aggregate inventory

- Design files excluding this manifest: 35
- Design-file words excluding this manifest: 46504
- Design-file bytes excluding this manifest: 331913
- Implementation tasks: 173
- Acceptance checklist items: 431
- Architecture decision records: 22

## File hashes

| File | Title | Words | Bytes | SHA-256 |
|---|---|---:|---:|---|
| `00_README.md` | Project Canopy | 672 | 5512 | `b973c39012590c50a326dbe86dac8db4ba904e43b5ec8ba48a3ed651601a9f9a` |
| `01_EXECUTIVE_SPEC.md` | Executive specification | 709 | 5180 | `9d2ba7df924758ffdd9d0fdd2d218b092cd2aa9833d098bb832a06c7f8a98d07` |
| `02_PARITY_BASELINE.md` | Functional parity baseline | 1373 | 8417 | `6d6bd04429a99a7d524fec6effcd56e6c1992cb06918527f29e2a71a6c3747f0` |
| `03_PRODUCT_REQUIREMENTS.md` | Product requirements | 941 | 6901 | `4e5021060a8bf9267b453c0dfa2cf579d5ff1bd918cdc29f213ba5c854fa3ce2` |
| `04_SYSTEM_ARCHITECTURE.md` | System architecture | 935 | 7053 | `fdc8a20f730b6ffbcf59f3becba0106ad3327bd6afa06fc0fdaaffeda2df86d7` |
| `05_REPOSITORY_AND_BUILD.md` | Repository and build design | 667 | 5041 | `33db221cf757a5e273975ff9a6d4d4c290654c9a3da20f0db27b6511dd9450f8` |
| `06_DATA_MODEL_AND_FILE_FORMATS.md` | Data model and file formats | 881 | 6667 | `cbc423ebfef9632eaa2c8fb337bc7805e58a0eed029a0e4c0340f080374e4c46` |
| `07_EVALUATION_ENGINE.md` | Procedural evaluation engine | 835 | 6326 | `849b31cf6dc281bd246dc224e50e62803aa3068ac862d9990837dec4333ab792` |
| `08_GENERATORS.md` | Generator specification | 923 | 6485 | `cef293706d3cd89cbe777f3f9dc61c7cb4847d6add6c3bfd398ab02dbbe3c041` |
| `09_BRANCH_MESHING.md` | Branch meshing and junctions | 1059 | 7738 | `cad8f29b2d790de3c403b62081c1e64917aaf970d2b609d779770abf77817a7d` |
| `10_FOLIAGE_VINES_AND_DETAILS.md` | Foliage, fronds, vines, and detail geometry | 1036 | 6958 | `8520572a70451eb1c8ebbaf6b49ecd7d08add6569672c99f8a885e163e6054d7` |
| `11_MANUAL_EDITING.md` | Manual editing and art-direction tools | 906 | 6185 | `51300e2ee9cd259aaa2922e36d80a38f2491b0460c6b78dd9f6e0d0b8d5fba85` |
| `12_MATERIALS_TEXTURES_AND_PHOTOGRAMMETRY.md` | Materials, textures, cutouts, and photogrammetry | 985 | 6752 | `e6535249aa803f48e7d6a320e3352cac7596bd48a2e81fcb4df86df24a94ee87` |
| `13_WIND_GROWTH_AND_SEASONS.md` | Wind, growth, seasons, and timeline | 933 | 6368 | `7f6f1feb5e7df9b33993f8519063a677d0349dc8bcfa6ac3041e167993982492` |
| `14_LOD_BILLBOARDS_AND_OPTIMIZATION.md` | LOD, billboards, collision, and optimization | 799 | 5512 | `58e9e99a860f13f125d8b5f61abea83de455de53cdb5e9af78c45c880e9f7867` |
| `15_VIEWPORT_AND_EDITOR_UX.md` | Viewport and editor user experience | 831 | 5698 | `0b4f0057d1a227ddda681d464055d07de10cd729cdc80d1734a899ccea674e93` |
| `16_EXPORT_PIPELINE.md` | Export pipeline | 846 | 6414 | `176031cb4c0174b97298711cd18d51c033e2ff89a58aa8b257c8a9a6c2d88d53` |
| `17_RUNTIME_SDK_AND_FOREST.md` | Runtime SDK and forest system | 1951 | 13950 | `09c852380a3583cfe2a4d47c32fa89d67943a16adbcfcc6b5264259498cf060e` |
| `18_ENGINE_AND_DCC_INTEGRATIONS.md` | Engine and DCC integrations | 1309 | 9111 | `9774d1d5d9758e694376aabbae6d361aeec585db38d429d36dfe748955ce135c` |
| `19_SCRIPTING_PLUGINS_AND_AUTOMATION.md` | Scripting, plugins, and automation | 1224 | 8931 | `8215529bb3f7b0460846422a7293f64b690ca956d7c8b35c6aa669c91a3f2f64` |
| `20_ASSET_LIBRARY_AND_VARIATION.md` | Asset library and variation system | 1111 | 8132 | `df3a28f14739da0d10c47ca82ab62b6f7eb51de5dee899ab568adc3c9b06a0b3` |
| `21_PUBLIC_API_CONTRACTS.md` | Public API contracts | 1076 | 8589 | `811fde520434d211ce9145b9026625059aa49b88aa62e261f60ff0bbd0f0d26f` |
| `22_NONFUNCTIONAL_REQUIREMENTS.md` | Nonfunctional requirements | 1244 | 8604 | `4b29b5329c72bb6735664a26f0f431faf01ed63f0998af860071161f4ef0faed` |
| `23_TEST_STRATEGY.md` | Test strategy | 1416 | 9672 | `9008a0907dcdda34062aac77515c5f8087101ec681755cebe7d96400436d654c` |
| `24_AGENT_IMPLEMENTATION_PLAYBOOK.md` | AI-agent implementation playbook | 1501 | 10158 | `54a12dd0e0a0b8c55f87232485d215e88fcdf05727b2b88c8d65da2d530b81b9` |
| `25_EPICS_AND_TASKS.md` | Epics and implementation tasks | 4544 | 31648 | `b897a4ed0a8eea08035f1a9e01bf1880934f24d4f8803cf955ebb55dd799d2e4` |
| `26_RELEASE_GATES.md` | Release gates | 1030 | 7210 | `589e0eead964a6f19dc2a7a1d9c5713fa88bf1edb0eb0d2bab9508b756688543` |
| `27_RISK_REGISTER.md` | Risk register | 1382 | 8360 | `bc380af9a2cdc0ba102023b4c3242a4697293e37515abcf993091f2bffde163d` |
| `28_CLEAN_ROOM_AND_LICENSING.md` | Clean-room implementation and licensing | 1024 | 7550 | `e59be529e565b5c6e6f5a6984e691c0340812bb2e6163343eae1d5bbb0c80171` |
| `29_ARCHITECTURE_DECISIONS.md` | Architecture decision records | 1257 | 10018 | `92b023ea1f0a9dde224c93509487c4d2bc46a180578d02e5b9f504ba43b32ea6` |
| `30_GLOSSARY.md` | Glossary | 1859 | 13547 | `219321d0c9239356c31b6b12177d5a0bf09b0e0e3a0c7f1cb0cbc397bbca97bf` |
| `31_OFFICIAL_SOURCES.md` | Official requirements sources | 792 | 7873 | `4f5804aae788c3708dc3982a75115d4dce240e5a037ecf17246d60f48a6b6794` |
| `32_ACCEPTANCE_CHECKLIST.md` | General-availability acceptance checklist | 4972 | 31057 | `4b5025c6aa04a093cd14465b9af14464bf075e36d0b62471c59bc8c1cb7762ea` |
| `33_AGENT_PROMPT_TEMPLATES.md` | AI-agent prompt templates | 2094 | 16821 | `65f2eac2857e58f50b0e009bb224ce73b4cfebf8fb110daa45e5db0488e10e91` |
| `34_BOOTSTRAP_BACKLOG.md` | Bootstrap backlog | 1387 | 11475 | `6d638bf72a7001955d81550816097400a9a916c2b4c7aa0ec2b1a6cc77accb2d` |

The manifest intentionally excludes its own digest to avoid self-reference. The distribution ZIP digest is reported alongside the downloadable artifact.

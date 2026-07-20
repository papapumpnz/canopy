# Performance baseline

Per `22_NONFUNCTIONAL_REQUIREMENTS.md`: baselines are **measured, not
inferred**, and every result records hardware and commit.

- **Commit**: 554f595 (+ benchmark suite in the same change)
- **Date**: 2026-07-21
- **Hardware**: Intel Core i7-1165G7 (4C/8T, 2.80 GHz base), 62 GB RAM, Linux 6.8
- **Toolchain**: GCC 13.3, `-O2` (`headless-release` preset), single-threaded
  (the deterministic single-worker mode; the parallel scheduler is future work)
- **Method**: `canopy-bench` — 1 warmup, then timed iterations until ≥5 runs
  and ≥1.2 s total; median and best wall time reported. Deterministic
  workloads. Rerun with:

  ```bash
  cmake --preset headless-release && cmake --build --preset headless-release
  build/headless-release/benchmarks/canopy-bench          # human output
  build/headless-release/benchmarks/canopy-bench --json   # machine output
  ```

## Micro/foundation

| Benchmark | Median | Best | Throughput |
|---|---:|---:|---|
| sha256_64MiB | 562.4 ms | 555.3 ms | 115 MiB/s |
| json_parse_2MB | 5.6 ms | 3.9 ms | 116 MB/s |
| json_write_canonical_2MB | 5.4 ms | 4.2 ms | 107 MB/s |
| curve_evaluate_1M (monotone cubic) | 20.2 ms | 16.6 ms | 60 M eval/s |
| spline_position_1M | 97.2 ms | 92.1 ms | 10 M query/s |

## Evaluation (spec scales)

"Standard asset" per the spec: ~50 generators, 25 materials (~146k production
triangles in this build of the synthetic benchmark document).

| Benchmark | Median | Best | Notes |
|---|---:|---:|---|
| evaluate_small_production | 0.07 ms | 0.07 ms | single trunk |
| evaluate_standard_production | 77.8 ms | 73.2 ms | ~2 M tri/s |
| evaluate_standard_draft | 71.5 ms | 60.8 ms | |
| evaluate_standard_windy | 109.0 ms | 97.6 ms | wind pass ≈ +31 ms |
| scaling probe (4× branch count) | — | — | **3.98× time — linear, no quadratic behavior** |

**Spec target check** — "property edit to full preview update: under 500 ms
for standard assets where cacheable": a *cold, uncached, single-threaded*
standard evaluation is **78 ms**, ~6× inside the target before any caching or
parallelism exists. The 22 target is met with headroom.

## Export and runtime

| Benchmark | Median | Best |
|---|---:|---:|
| merge_by_material_standard | 35.6 ms | 31.5 ms |
| write_glb_standard | 400.0 ms | 389.2 ms |
| compile_canopyrt_standard (3 LODs, incl. 3 evaluations) | 635.7 ms | 632.3 ms |
| rt_load_standard | 232.9 ms | 206.7 ms |
| forest_scatter_100k | 3.0 ms | 2.2 ms (44 M inst/s) |
| forest_select_100k (cull + LOD) | 1.0 ms | 0.8 ms (128 M inst/s) |

Forest ops leave enormous headroom against the runtime spec floor (100k
visible instances): full CPU cull + LOD selection of 100k instances costs
under 1 ms/frame.

## Real sample documents (release CLI, cold process incl. load + I/O)

| Document | evaluate | export GLB | compile .canopyrt |
|---|---:|---:|---:|
| CoastalOak (38k tri) | 0.06 s | 0.13 s | 0.23 s |
| AlpineFir (89k tri, 1,651 nodes) | 0.16 s | 0.29 s | 0.54 s |
| WeepingWillow | 0.05 s | | |
| IslandPalm | 0.02 s | | |
| RiverBirch | 0.04 s | | |
| NebulaSporeTree | 0.03 s | | |

## Known hotspots (optimization backlog, in impact order)

1. **SHA-256 (115 MiB/s scalar)** dominates GLB export, `.canopyrt` compile
   and load (per-section checksums): ~60% of `rt_load_standard`. Options:
   SHA-NI intrinsics behind a runtime dispatch, or a cheaper non-crypto
   digest for section checksums (would be an ADR — the format stores the
   hash kind implicitly today).
2. **spline_position_at** does a binary search per query (10 M/s); batch
   sampling in `build_branch` could walk the arc-length table linearly.
3. **Evaluation is single-threaded** by design for now; the deterministic
   parallel scheduler (07) is the structural win for hero/forest scales.

None of these block current targets; they are recorded so future changes are
measured against this baseline rather than guessed at.

// Canopy performance baselines (22_NONFUNCTIONAL_REQUIREMENTS.md).
//
// In-house harness (ADR-0001: no deps): per benchmark, one warmup run, then
// timed iterations until >= min_iterations and >= min_total time; reports the
// median and best wall time plus a derived throughput. Workloads are
// deterministic; results are for the release preset only.
//
// Usage: canopy-bench [--json] [--filter substring]
#include "canopy/document/document.hpp"
#include "canopy/evaluation/evaluate.hpp"
#include "canopy/export/gltf_export.hpp"
#include "canopy/export/mesh_merge.hpp"
#include "canopy/export/rt_compile.hpp"
#include "canopy/foundation/curve.hpp"
#include "canopy/foundation/hash.hpp"
#include "canopy/foundation/json.hpp"
#include "canopy/geometry/spline.hpp"
#include "canopy/runtime/forest.hpp"
#include "canopy/runtime/model.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace {

using namespace canopy;
using Clock = std::chrono::steady_clock;

struct BenchResult {
    std::string name;
    std::size_t iterations = 0;
    double median_ms = 0.0;
    double best_ms = 0.0;
    std::string throughput;
};

std::vector<BenchResult> g_results;

// A sink the optimizer cannot remove.
volatile double g_sink = 0.0;

void run_bench(const std::string& name, const std::function<void()>& body,
               const std::function<std::string(double best_ms)>& throughput = {},
               std::size_t min_iterations = 5, double min_total_ms = 1200.0) {
    body(); // warmup
    std::vector<double> times;
    double total = 0.0;
    while (times.size() < min_iterations || total < min_total_ms) {
        const auto start = Clock::now();
        body();
        const double ms =
            std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        times.push_back(ms);
        total += ms;
        if (times.size() > 200) {
            break;
        }
    }
    std::sort(times.begin(), times.end());
    BenchResult result;
    result.name = name;
    result.iterations = times.size();
    result.median_ms = times[times.size() / 2];
    result.best_ms = times.front();
    if (throughput) {
        result.throughput = throughput(result.best_ms);
    }
    g_results.push_back(result);
    std::fprintf(stderr, "  %-32s median %9.2f ms  best %9.2f ms  %s\n", name.c_str(),
                 result.median_ms, result.best_ms, result.throughput.c_str());
}

std::string format_rate(double units, double best_ms, const char* unit) {
    const double per_second = units / (best_ms / 1000.0);
    if (per_second >= 1e6) {
        return std::to_string(std::int64_t(per_second / 1e6)) + "M " + unit + "/s";
    }
    if (per_second >= 1e3) {
        return std::to_string(std::int64_t(per_second / 1e3)) + "k " + unit + "/s";
    }
    return std::to_string(std::int64_t(per_second)) + " " + unit + "/s";
}

Uuid make_uuid(std::uint32_t n) {
    char text[37];
    std::snprintf(text, sizeof text, "%08x-1111-4222-8333-%012x", n, n);
    return *Uuid::parse(text);
}

// --- documents at the spec's scales ----------------------------------------

doc::GeneratorInstance branch_generator(std::uint32_t id, const Uuid& parent,
                                        double spacing, double angle, double length,
                                        double radius, std::uint32_t segments) {
    doc::GeneratorInstance generator;
    generator.id = make_uuid(id);
    generator.type = "canopy.branch";
    generator.name = "G" + std::to_string(id);
    generator.parent = parent;
    generator.properties.emplace("generation.mode", json::Value("interval"));
    generator.properties.emplace("generation.spacing.relative", json::Value(spacing));
    generator.properties.emplace("generation.first", json::Value(0.25));
    generator.properties.emplace("generation.angle.degrees", json::Value(angle));
    generator.properties.emplace("generation.angle.variance.degrees", json::Value(12));
    generator.properties.emplace("spine.length.absolute", json::Value(length));
    generator.properties.emplace("spine.length.variance.relative", json::Value(0.2));
    generator.properties.emplace("spine.radius.absolute", json::Value(radius));
    generator.properties.emplace("spine.wander.degrees", json::Value(8));
    generator.properties.emplace("mesh.radial_segments", json::Value(std::int64_t(segments)));
    return generator;
}

// "Standard asset": ~50 generators, 25 materials, ~250k production triangles.
doc::Document standard_document() {
    doc::Document document;
    document.manifest.document_id = make_uuid(0xA0);
    document.manifest.name = "BenchStandard";
    document.manifest.seed = 987654321;

    for (std::uint32_t m = 0; m < 25; ++m) {
        doc::Material material;
        material.id = make_uuid(0xB00 + m);
        material.name = "material_" + std::to_string(m);
        material.two_sided = m > 0;
        if (m > 0) {
            material.card_region = {0.0, 0.0, 0.5, 0.5};
        }
        document.materials.push_back(material);
    }

    doc::GeneratorInstance tree;
    tree.id = make_uuid(1);
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);

    doc::GeneratorInstance trunk;
    trunk.id = make_uuid(2);
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = make_uuid(1);
    trunk.properties.emplace("spine.length.absolute", json::Value(6.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.3));
    trunk.properties.emplace("mesh.radial_segments", json::Value(16));
    trunk.properties.emplace("material.bark", json::Value(make_uuid(0xB00).str()));
    document.generators.push_back(trunk);

    // 16 chains of bough → branch → leaf batch = 48 generators (+2 above).
    for (std::uint32_t chain = 0; chain < 16; ++chain) {
        auto bough = branch_generator(10 + chain * 3, make_uuid(2), 0.11,
                                      45.0 + double(chain % 5) * 8.0, 2.4, 0.08, 8);
        auto branch = branch_generator(11 + chain * 3, bough.id, 0.14, 50.0, 1.1, 0.03, 6);
        doc::GeneratorInstance leaves;
        leaves.id = make_uuid(12 + chain * 3);
        leaves.type = "canopy.batched_leaf";
        leaves.name = "Leaves" + std::to_string(chain);
        leaves.parent = branch.id;
        leaves.properties.emplace("generation.spacing.relative", json::Value(0.09));
        leaves.properties.emplace("generation.leaves_per_point", json::Value(3));
        leaves.properties.emplace("leaf.length.absolute", json::Value(0.09));
        leaves.properties.emplace(
            "material.leaf", json::Value(make_uuid(0xB01 + (chain % 24)).str()));
        document.generators.push_back(bough);
        document.generators.push_back(branch);
        document.generators.push_back(leaves);
    }
    return document;
}

doc::Document small_document() {
    doc::Document document;
    document.manifest.document_id = make_uuid(0xA1);
    document.manifest.name = "BenchSmall";
    document.manifest.seed = 42;
    doc::GeneratorInstance tree;
    tree.id = make_uuid(1);
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);
    doc::GeneratorInstance trunk;
    trunk.id = make_uuid(2);
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = make_uuid(1);
    trunk.properties.emplace("spine.length.absolute", json::Value(4.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.25));
    trunk.properties.emplace("mesh.radial_segments", json::Value(12));
    document.generators.push_back(trunk);
    return document;
}

// Scaling probe for quadratic behavior (22: "avoid quadratic behavior in
// ordinary branch counts"): the same shape at 1x and 4x the branch count
// should evaluate in roughly 4x the time, not 16x.
doc::Document scaling_document(double spacing) {
    doc::Document document;
    document.manifest.document_id = make_uuid(0xA2);
    document.manifest.name = "BenchScaling";
    document.manifest.seed = 7;
    doc::GeneratorInstance tree;
    tree.id = make_uuid(1);
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);
    doc::GeneratorInstance trunk;
    trunk.id = make_uuid(2);
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = make_uuid(1);
    trunk.properties.emplace("spine.length.absolute", json::Value(8.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.3));
    document.generators.push_back(trunk);
    document.generators.push_back(
        branch_generator(3, make_uuid(2), spacing, 55.0, 1.2, 0.04, 6));
    return document;
}

} // namespace

int main(int argc, char** argv) {
    bool json_output = false;
    std::string filter;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--json") {
            json_output = true;
        } else if (arg == "--filter" && i + 1 < argc) {
            filter = argv[++i];
        }
    }
    auto enabled = [&filter](const std::string& name) {
        return filter.empty() || name.find(filter) != std::string::npos;
    };
    const auto temp_dir = std::filesystem::temp_directory_path() / "canopy-bench";
    std::filesystem::create_directories(temp_dir);

    // --- foundation primitives ---------------------------------------------
    if (enabled("sha256")) {
        const std::string payload(64 * 1024 * 1024, 'x');
        run_bench(
            "sha256_64MiB", [&] { g_sink = double(sha256(payload).low64()); },
            [](double ms) { return format_rate(64.0, ms, "MiB"); }, 3);
    }
    if (enabled("json")) {
        // ~2 MB canonical properties-style document.
        json::Object root;
        for (int g = 0; g < 400; ++g) {
            json::Object properties;
            for (int p = 0; p < 40; ++p) {
                properties.emplace("property." + std::to_string(p),
                                   json::Value(0.001 * double(g * 40 + p)));
            }
            root.emplace("generator-" + std::to_string(g), std::move(properties));
        }
        const std::string text = json::write_canonical(json::Value(root)).value();
        const double mb = double(text.size()) / 1e6;
        run_bench(
            "json_parse_2MB",
            [&] { g_sink = double(json::parse(text).value().as_object().size()); },
            [mb](double ms) { return format_rate(mb, ms, "MB"); });
        auto parsed = json::parse(text).value();
        run_bench(
            "json_write_canonical_2MB",
            [&] { g_sink = double(json::write_canonical(parsed).value().size()); },
            [mb](double ms) { return format_rate(mb, ms, "MB"); });
    }
    if (enabled("curve")) {
        auto curve = Curve::create({{0.0, 0.0, 0, 0, {}},
                                    {0.3, 0.8, 0, 0, {}},
                                    {0.7, 0.2, 0, 0, {}},
                                    {1.0, 1.0, 0, 0, {}}},
                                   CurveInterpolation::monotone_cubic)
                         .value();
        run_bench(
            "curve_evaluate_1M",
            [&] {
                double total = 0.0;
                for (int i = 0; i < 1000000; ++i) {
                    total += curve.evaluate(double(i) * 1e-6);
                }
                g_sink = total;
            },
            [](double ms) { return format_rate(1e6, ms, "eval"); });
    }
    if (enabled("spline")) {
        auto spline = geo::Spline::create(
                          {{0, 0, 0}, {0.2, 2, 0.1}, {0.1, 4, 0.3}, {0.4, 6, 0.2}})
                          .value();
        run_bench(
            "spline_position_1M",
            [&] {
                double total = 0.0;
                for (int i = 0; i < 1000000; ++i) {
                    total += spline.position_at(double(i) * 1e-6).y;
                }
                g_sink = total;
            },
            [](double ms) { return format_rate(1e6, ms, "query"); });
    }

    // --- evaluation at the spec's scales -----------------------------------
    const doc::Document small_doc = small_document();
    const doc::Document standard_doc = standard_document();
    std::size_t standard_triangles = 0;
    if (enabled("evaluate")) {
        run_bench(
            "evaluate_small_production",
            [&] {
                auto model = eval::evaluate(small_doc, eval::EvaluationProfile::production());
                g_sink = double(model.value().total_triangles());
            },
            [&](double ms) {
                auto model = eval::evaluate(small_doc, eval::EvaluationProfile::production());
                return format_rate(double(model.value().total_triangles()), ms, "tri");
            });
        {
            auto model = eval::evaluate(standard_doc, eval::EvaluationProfile::production());
            standard_triangles = model.value().total_triangles();
        }
        run_bench(
            "evaluate_standard_production",
            [&] {
                auto model =
                    eval::evaluate(standard_doc, eval::EvaluationProfile::production());
                g_sink = double(model.value().total_triangles());
            },
            [&](double ms) {
                return format_rate(double(standard_triangles), ms, "tri");
            },
            5, 2000.0);
        run_bench("evaluate_standard_draft", [&] {
            auto model = eval::evaluate(standard_doc, eval::EvaluationProfile::draft());
            g_sink = double(model.value().total_triangles());
        });
        run_bench("evaluate_standard_windy", [&] {
            eval::TimelineSample sample;
            sample.wind_strength = 0.8;
            sample.time_s = 1.5;
            auto model = eval::evaluate(standard_doc, eval::EvaluationProfile::production(),
                                        sample);
            g_sink = double(model.value().total_triangles());
        });
    }
    if (enabled("scaling")) {
        // Quadratic-behavior probe: 4x branches should be ~4x the time.
        const doc::Document coarse = scaling_document(0.02);
        const doc::Document dense = scaling_document(0.005);
        double coarse_ms = 0.0;
        double dense_ms = 0.0;
        run_bench("scaling_branches_1x", [&] {
            const auto start = Clock::now();
            auto model = eval::evaluate(coarse, eval::EvaluationProfile::preview());
            coarse_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            g_sink = double(model.value().nodes.size());
        });
        run_bench("scaling_branches_4x", [&] {
            const auto start = Clock::now();
            auto model = eval::evaluate(dense, eval::EvaluationProfile::preview());
            dense_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            g_sink = double(model.value().nodes.size());
        });
        std::fprintf(stderr, "  scaling ratio (4x branches): %.2fx time\n",
                     dense_ms / std::max(coarse_ms, 1e-9));
    }

    // --- export ------------------------------------------------------------
    if (enabled("export")) {
        auto model = eval::evaluate(standard_doc, eval::EvaluationProfile::production());
        run_bench("merge_by_material_standard", [&] {
            g_sink = double(exp::merge_by_material(standard_doc, model.value()).size());
        });
        exp::ExportPreset preset;
        preset.format = "gltf";
        run_bench("write_glb_standard", [&] {
            auto manifest =
                exp::write_glb(standard_doc, model.value(), preset, temp_dir / "bench");
            g_sink = double(manifest.value().vertex_count);
        });
        std::vector<eval::EvaluatedModel> lods;
        for (const auto& profile : {eval::EvaluationProfile::production(),
                                    eval::EvaluationProfile::preview(),
                                    eval::EvaluationProfile::draft()}) {
            lods.push_back(std::move(eval::evaluate(standard_doc, profile)).value());
        }
        run_bench("compile_canopyrt_standard", [&] {
            auto manifest = exp::write_canopyrt(standard_doc, lods, temp_dir / "bench-rt");
            g_sink = double(manifest.value().lod_count);
        });
    }

    // --- runtime -----------------------------------------------------------
    if (enabled("runtime")) {
        {
            auto model = eval::evaluate(standard_doc, eval::EvaluationProfile::production());
            std::vector<eval::EvaluatedModel> lods;
            lods.push_back(std::move(model).value());
            lods.push_back(
                std::move(eval::evaluate(standard_doc, eval::EvaluationProfile::preview()))
                    .value());
            lods.push_back(
                std::move(eval::evaluate(standard_doc, eval::EvaluationProfile::draft()))
                    .value());
            (void)exp::write_canopyrt(standard_doc, lods, temp_dir / "bench-rt");
        }
        run_bench("rt_load_standard", [&] {
            auto model = rt::load_model(temp_dir / "bench-rt.canopyrt");
            g_sink = double(model.value().lods.size());
        });
        auto loaded = rt::load_model(temp_dir / "bench-rt.canopyrt").value();
        rt::Forest forest;
        forest.add_model(&loaded);
        run_bench(
            "forest_scatter_100k",
            [&] {
                (void)forest.scatter(7, 100000, 4000.0, 0.0);
                g_sink = double(forest.instances().size());
            },
            [](double ms) { return format_rate(1e5, ms, "instance"); }, 3);
        (void)forest.scatter(7, 100000, 4000.0, 0.0);
        run_bench(
            "forest_select_100k",
            [&] {
                g_sink = double(forest.select(Vec3{0, 2, 0}, 1500.0).size());
            },
            [](double ms) { return format_rate(1e5, ms, "instance"); });
    }

    // --- report ------------------------------------------------------------
    if (json_output) {
        json::Array results;
        for (const auto& result : g_results) {
            json::Object entry;
            entry.emplace("name", result.name);
            entry.emplace("iterations", std::int64_t(result.iterations));
            entry.emplace("median_ms", result.median_ms);
            entry.emplace("best_ms", result.best_ms);
            if (!result.throughput.empty()) {
                entry.emplace("throughput", result.throughput);
            }
            results.push_back(std::move(entry));
        }
        json::Object root;
        root.emplace("benchmarks", std::move(results));
        auto text = json::write_canonical(json::Value(std::move(root)));
        if (text.ok()) {
            std::fputs(text.value().c_str(), stdout);
        }
    }
    return 0;
}

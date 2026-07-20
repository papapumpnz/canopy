// canopy-runtime-demo: the CPU-forest reference path (17_RUNTIME_SDK,
// ADR-0008). Loads compiled .canopyrt models — no authoring code linked —
// scatters a deterministic forest, distance-culls and LOD-selects from a
// camera, and bakes the visible set to OBJ/MTL for diagnostic rendering.
// Stats go to stdout as canonical JSON.
#include "canopy/foundation/json.hpp"
#include "canopy/runtime/forest.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {

using namespace canopy;

int fail(const Diagnostic& diagnostic) {
    std::fputs(format_human(diagnostic).c_str(), stderr);
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> model_paths;
    std::string out_base = "forest";
    std::uint64_t seed = 7;
    std::uint32_t count = 40;
    double extent = 30.0;
    double spacing = 3.0;
    double max_distance = 0.0; // 0 = unlimited
    Vec3 camera{0.0, 2.0, 60.0};

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--out" && i + 1 < argc) {
            out_base = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = std::strtoull(argv[++i], nullptr, 10);
        } else if (arg == "--count" && i + 1 < argc) {
            count = std::uint32_t(std::strtoul(argv[++i], nullptr, 10));
        } else if (arg == "--extent" && i + 1 < argc) {
            extent = std::atof(argv[++i]);
        } else if (arg == "--spacing" && i + 1 < argc) {
            spacing = std::atof(argv[++i]);
        } else if (arg == "--max-distance" && i + 1 < argc) {
            max_distance = std::atof(argv[++i]);
        } else if (arg == "--camera" && i + 3 < argc) {
            camera.x = std::atof(argv[++i]);
            camera.y = std::atof(argv[++i]);
            camera.z = std::atof(argv[++i]);
        } else if (!arg.empty() && arg[0] == '-') {
            std::fprintf(stderr,
                         "usage: canopy-runtime-demo <model.canopyrt>... [--out base] "
                         "[--seed n] [--count n] [--extent m] [--spacing m] "
                         "[--camera x y z] [--max-distance m]\n");
            return 1;
        } else {
            model_paths.emplace_back(arg);
        }
    }
    if (model_paths.empty()) {
        std::fputs("at least one .canopyrt model is required\n", stderr);
        return 1;
    }

    std::vector<rt::RtModel> models;
    models.reserve(model_paths.size());
    for (const std::string& path : model_paths) {
        auto loaded = rt::load_model(path);
        if (!loaded.ok()) {
            return fail(loaded.error());
        }
        models.push_back(std::move(loaded).value());
    }

    rt::Forest forest;
    for (const rt::RtModel& model : models) {
        forest.add_model(&model);
    }
    if (auto scattered = forest.scatter(seed, count, extent, spacing); !scattered.ok()) {
        return fail(scattered.error());
    }
    const auto visible = forest.select(camera, max_distance);

    // Bake visible instances to OBJ/MTL. Materials are namespaced per model
    // so same-named materials keep their own colors.
    std::string mtl;
    mtl += "# canopy-runtime-demo forest bake\n";
    for (std::size_t m = 0; m < models.size(); ++m) {
        for (const rt::RtMaterial& material : models[m].materials) {
            mtl += "newmtl m" + std::to_string(m) + "_" + material.name + "\nKd ";
            mtl += json::format_double(material.color[0]);
            mtl += ' ';
            mtl += json::format_double(material.color[1]);
            mtl += ' ';
            mtl += json::format_double(material.color[2]);
            mtl += '\n';
        }
    }

    std::string obj;
    obj += "# canopy-runtime-demo forest bake\nmtllib " + out_base + ".mtl\n";
    // Note: mtllib expects a filename, not a path — strip directories.
    {
        const auto slash = out_base.find_last_of('/');
        if (slash != std::string::npos) {
            obj = "# canopy-runtime-demo forest bake\nmtllib " + out_base.substr(slash + 1) +
                  ".mtl\n";
        }
    }

    std::size_t vertex_base = 1;
    std::size_t baked_triangles = 0;
    std::map<std::uint32_t, std::size_t> lod_histogram;
    for (const rt::VisibleInstance& item : visible) {
        const rt::ForestInstance& instance = *item.instance;
        const rt::RtModel& model = *forest.models()[instance.model];
        const rt::RtLod& lod = model.lods[item.lod];
        ++lod_histogram[item.lod];
        const double c = std::cos(instance.yaw);
        const double s = std::sin(instance.yaw);
        for (std::size_t v = 0; v < lod.vertex_count(); ++v) {
            const float* data = lod.vertices.data() + v * 8;
            const double x = double(data[0]) * instance.scale;
            const double y = double(data[1]) * instance.scale;
            const double z = double(data[2]) * instance.scale;
            const double world_x = c * x + s * z + instance.position.x;
            const double world_z = -s * x + c * z + instance.position.z;
            obj += "v ";
            obj += json::format_double(world_x);
            obj += ' ';
            obj += json::format_double(y + instance.position.y);
            obj += ' ';
            obj += json::format_double(world_z);
            obj += '\n';
        }
        for (const rt::RtPrimitive& primitive : lod.primitives) {
            obj += "usemtl m" + std::to_string(instance.model) + "_" +
                   model.materials[primitive.material].name + "\n";
            for (std::uint32_t i = 0; i < primitive.index_count; i += 3) {
                obj += 'f';
                for (std::uint32_t k = 0; k < 3; ++k) {
                    obj += ' ';
                    obj += json::format_int(std::int64_t(
                        vertex_base + lod.indices[primitive.index_offset + i + k]));
                }
                obj += '\n';
            }
            baked_triangles += primitive.index_count / 3;
        }
        vertex_base += lod.vertex_count();
    }

    {
        std::ofstream stream(out_base + ".obj", std::ios::binary | std::ios::trunc);
        stream << obj;
        if (!stream) {
            std::fputs("failed to write forest OBJ\n", stderr);
            return 2;
        }
    }
    {
        std::ofstream stream(out_base + ".mtl", std::ios::binary | std::ios::trunc);
        stream << mtl;
    }

    json::Object stats;
    stats.emplace("ok", true);
    stats.emplace("models", std::int64_t(models.size()));
    stats.emplace("instances", std::int64_t(forest.instances().size()));
    stats.emplace("visible", std::int64_t(visible.size()));
    stats.emplace("baked_triangles", std::int64_t(baked_triangles));
    json::Object lods_json;
    for (const auto& [lod, instances] : lod_histogram) {
        lods_json.emplace("lod" + std::to_string(lod), std::int64_t(instances));
    }
    stats.emplace("lod_histogram", std::move(lods_json));
    auto text = json::write_canonical(json::Value(std::move(stats)));
    if (text.ok()) {
        std::fputs(text.value().c_str(), stdout);
    }
    return 0;
}

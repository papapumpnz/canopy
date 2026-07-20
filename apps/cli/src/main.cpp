// canopy-cli: deterministic headless validate / evaluate / export (backlog
// B-030, bootstrap integration gate commands).
//
// Exit codes: 0 success, 1 usage error, 2 validation/evaluation/export
// failure. Diagnostics go to stderr (human) or stdout (--json), so JSON
// output stays machine-parseable.
#include "canopy/document/project_io.hpp"
#include "canopy/evaluation/evaluate.hpp"
#include "canopy/export/obj_export.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace canopy;

void print_usage() {
    std::fputs(
        "Project Canopy CLI\n"
        "\n"
        "usage:\n"
        "  canopy-cli validate <project.canopyproj> [--json]\n"
        "  canopy-cli evaluate <project.canopyproj> [--profile <name>] [timeline] [--json]\n"
        "  canopy-cli export   <project.canopyproj> --preset <preset.json> --out <base>\n"
        "                      [timeline] [--json]\n"
        "\n"
        "profiles: draft, preview, production (default: production)\n"
        "timeline: --time <s> --growth <0..1> --season <0..1>\n"
        "          --wind-strength <0..1> --wind-direction <deg> --gust <0..1>\n",
        stderr);
}

int report_failure(const Diagnostic& diagnostic, bool json_output) {
    if (json_output) {
        json::Object envelope;
        envelope.emplace("ok", false);
        auto parsed = json::parse(format_json(diagnostic), "diagnostic");
        envelope.emplace("error", parsed.ok() ? std::move(parsed).value() : json::Value(nullptr));
        auto text = json::write_canonical(json::Value(std::move(envelope)));
        std::fputs(text.ok() ? text.value().c_str() : "{\"ok\": false}\n", stdout);
    } else {
        std::fputs(format_human(diagnostic).c_str(), stderr);
    }
    return 2;
}

void print_success_json(json::Object payload) {
    payload.emplace("ok", true);
    auto text = json::write_canonical(json::Value(std::move(payload)));
    if (text.ok()) {
        std::fputs(text.value().c_str(), stdout);
    }
}

json::Array warnings_to_json(const std::vector<Diagnostic>& warnings) {
    json::Array out;
    for (const auto& warning : warnings) {
        auto parsed = json::parse(format_json(warning), "warning");
        if (parsed.ok()) {
            out.push_back(std::move(parsed).value());
        }
    }
    return out;
}

struct CommonArgs {
    std::string project;
    std::string profile = "production";
    std::string preset;
    std::string out;
    eval::TimelineSample sample;
    bool json_output = false;
    bool valid = false;
};

CommonArgs parse_args(int argc, char** argv) {
    CommonArgs args;
    std::vector<std::string_view> positional;
    for (int i = 2; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--json") {
            args.json_output = true;
        } else if (arg == "--profile" && i + 1 < argc) {
            args.profile = argv[++i];
        } else if (arg == "--preset" && i + 1 < argc) {
            args.preset = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            args.out = argv[++i];
        } else if (arg == "--time" && i + 1 < argc) {
            args.sample.time_s = std::atof(argv[++i]);
        } else if (arg == "--growth" && i + 1 < argc) {
            args.sample.growth = std::atof(argv[++i]);
        } else if (arg == "--season" && i + 1 < argc) {
            args.sample.season = std::atof(argv[++i]);
        } else if (arg == "--wind-strength" && i + 1 < argc) {
            args.sample.wind_strength = std::atof(argv[++i]);
        } else if (arg == "--wind-direction" && i + 1 < argc) {
            args.sample.wind_direction_deg = std::atof(argv[++i]);
        } else if (arg == "--gust" && i + 1 < argc) {
            args.sample.gust = std::atof(argv[++i]);
        } else if (!arg.empty() && arg[0] == '-') {
            std::fprintf(stderr, "unknown option: %.*s\n", int(arg.size()), arg.data());
            return args;
        } else {
            positional.push_back(arg);
        }
    }
    if (positional.size() != 1) {
        std::fputs("expected exactly one project path\n", stderr);
        return args;
    }
    args.project = std::string(positional.front());
    args.valid = true;
    return args;
}

int run_validate(const CommonArgs& args) {
    auto document = doc::load_project(args.project);
    if (!document.ok()) {
        return report_failure(document.error(), args.json_output);
    }
    auto files = doc::render_project_files(document.value());
    if (!files.ok()) {
        return report_failure(files.error(), args.json_output);
    }
    const ContentHash hash = doc::document_hash(files.value());
    if (args.json_output) {
        json::Object payload;
        payload.emplace("command", "validate");
        payload.emplace("document_id", document.value().manifest.document_id.str());
        payload.emplace("document_hash", hash.hex());
        payload.emplace("generator_count", std::int64_t(document.value().generators.size()));
        print_success_json(std::move(payload));
    } else {
        std::printf("valid: %s\n", args.project.c_str());
        std::printf("document_id: %s\n", document.value().manifest.document_id.str().c_str());
        std::printf("document_hash: %s\n", hash.hex().c_str());
        std::printf("generators: %zu\n", document.value().generators.size());
    }
    return 0;
}

int run_evaluate(const CommonArgs& args) {
    auto document = doc::load_project(args.project);
    if (!document.ok()) {
        return report_failure(document.error(), args.json_output);
    }
    auto profile = eval::EvaluationProfile::by_name(args.profile);
    if (!profile.ok()) {
        return report_failure(profile.error(), args.json_output);
    }
    auto model = eval::evaluate(document.value(), profile.value(), args.sample);
    if (!model.ok()) {
        return report_failure(model.error(), args.json_output);
    }
    auto files = doc::render_project_files(document.value());
    if (!files.ok()) {
        return report_failure(files.error(), args.json_output);
    }
    if (args.json_output) {
        json::Object payload;
        payload.emplace("command", "evaluate");
        payload.emplace("profile", profile.value().name);
        payload.emplace("document_hash", doc::document_hash(files.value()).hex());
        payload.emplace("node_count", std::int64_t(model.value().nodes.size()));
        payload.emplace("vertex_count", std::int64_t(model.value().total_vertices()));
        payload.emplace("triangle_count", std::int64_t(model.value().total_triangles()));
        payload.emplace("model_hash", SemanticId{model.value().model_hash()}.str());
        payload.emplace("model_topology_hash",
                        SemanticId{model.value().model_topology_hash()}.str());
        payload.emplace("warnings", warnings_to_json(model.value().warnings));
        print_success_json(std::move(payload));
    } else {
        std::printf("profile: %s\n", profile.value().name.c_str());
        std::printf("nodes: %zu\n", model.value().nodes.size());
        std::printf("vertices: %zu\n", model.value().total_vertices());
        std::printf("triangles: %zu\n", model.value().total_triangles());
        std::printf("model_hash: %s\n", SemanticId{model.value().model_hash()}.str().c_str());
        std::printf("model_topology_hash: %s\n",
                    SemanticId{model.value().model_topology_hash()}.str().c_str());
        for (const auto& warning : model.value().warnings) {
            std::fputs(format_human(warning).c_str(), stderr);
        }
    }
    return 0;
}

int run_export(const CommonArgs& args) {
    if (args.preset.empty() || args.out.empty()) {
        std::fputs("export requires --preset and --out\n", stderr);
        return 1;
    }
    auto document = doc::load_project(args.project);
    if (!document.ok()) {
        return report_failure(document.error(), args.json_output);
    }
    auto preset = exp::ExportPreset::load(args.preset);
    if (!preset.ok()) {
        return report_failure(preset.error(), args.json_output);
    }
    auto profile = eval::EvaluationProfile::by_name(preset.value().profile);
    if (!profile.ok()) {
        return report_failure(profile.error(), args.json_output);
    }
    auto model = eval::evaluate(document.value(), profile.value(), args.sample);
    if (!model.ok()) {
        return report_failure(model.error(), args.json_output);
    }
    auto manifest = exp::write_obj(document.value(), model.value(), preset.value(), args.out);
    if (!manifest.ok()) {
        return report_failure(manifest.error(), args.json_output);
    }
    if (args.json_output) {
        json::Object payload;
        payload.emplace("command", "export");
        payload.emplace("obj_file", manifest.value().obj_file);
        payload.emplace("obj_sha256", manifest.value().obj_sha256.hex());
        payload.emplace("model_hash", SemanticId{manifest.value().model_hash}.str());
        payload.emplace("node_count", std::int64_t(manifest.value().node_count));
        payload.emplace("vertex_count", std::int64_t(manifest.value().vertex_count));
        payload.emplace("triangle_count", std::int64_t(manifest.value().triangle_count));
        payload.emplace("warnings", warnings_to_json(model.value().warnings));
        print_success_json(std::move(payload));
    } else {
        std::printf("exported: %s (%zu vertices, %zu triangles)\n",
                    manifest.value().obj_file.c_str(), manifest.value().vertex_count,
                    manifest.value().triangle_count);
        std::printf("obj_sha256: %s\n", manifest.value().obj_sha256.hex().c_str());
        for (const auto& warning : model.value().warnings) {
            std::fputs(format_human(warning).c_str(), stderr);
        }
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    const std::string_view command = argv[1];
    if (command == "--help" || command == "-h" || command == "help") {
        print_usage();
        return 0;
    }
    const CommonArgs args = parse_args(argc, argv);
    if (!args.valid) {
        print_usage();
        return 1;
    }
    if (command == "validate") {
        return run_validate(args);
    }
    if (command == "evaluate") {
        return run_evaluate(args);
    }
    if (command == "export") {
        return run_export(args);
    }
    std::fprintf(stderr, "unknown command: %.*s\n", int(command.size()), command.data());
    print_usage();
    return 1;
}

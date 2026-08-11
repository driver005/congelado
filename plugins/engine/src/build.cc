#include <congelado/abi.h>
#include <cstdio>

import std;
import core_router;
import core_plugin;
import serde;
import congelado_heart;
import engine;
import utils_openapi;

// Build-time-only tool: registers the engine's own routes into a throwaway RouterContext
// (which, via ApiRoute/ApiRouter's constructors, populates utils::openapi::Registry as a side
// effect — see engine::register_routes' own doc comments), then writes the engine's OpenAPI
// document and feeds it into IOpenApiGenerator's generate_client_sdk() to produce the engine's
// typed client SDK (congelado_api::*, consumed by engine_worker_lib so the worker can call into
// the engine's HTTP API). The worker's own side of this split lives in
// plugins/engine/worker/build.cc — see that file for the symmetric worker-only tool. Never
// linked into congelado_lib, the engine plugin, or the congelado_worker binary itself — xmake's
// generic build.cc feature runs this (no args — takes none) before engine_lib's real sources
// compile.
//
// Output lands inside generated/engine/ (plugins/engine/generated/engine/, a sibling of this
// file's own src/ directory), so that engine_worker_lib can add_files() the result directly.
// Paths are relative to this file's own directory (curdir is pinned there by
// xmake/modules/build_tool.lua's os.execv(... {curdir = target:scriptdir() .. "/src"})), hence
// "../generated/engine/".
int main() {
    // Force-load the JSON format plugin before anything below calls serde::Ser::serialize —
    // the openapi_generator plugin's write_document() dispatches through Ser (runtime format
    // lookup) same as everywhere else, instead of calling serde::Json::encode directly; this
    // build-time tool has no plugin-loading of its own otherwise, so it needs its own tiny
    // bootstrap. Opens libjson_plugin.so and libopenapi_generator.so by name each (not the whole
    // plugins directory via open_all()) — some plugins (e.g. postgres, which links a system
    // GSSAPI lib) can fail to dlopen in a given environment, and this tool has no business
    // caring about any plugin besides the two it actually needs. Both build as their own
    // standalone xmake projects (plugins/json/, plugins/openapi_generator/) rather than targets
    // inside this same invocation — their .so's land in the shared cross-project plugin output
    // dir (repo_root/build/plugins), which the orchestrator task guarantees is built before
    // engine's own project runs this tool.
    serde::SerdeFormatRegistry format_registry;
    serde::SerdeFormatRegistry::set_active(&format_registry);

    // Force-load the OpenAPI generator plugin too — utils::openapi::Generator and
    // congelado::client::Generator both moved out of this build-time tool's direct reach and
    // into plugins/openapi_generator/ (see interfaces::IOpenApiGenerator), so this tool now
    // needs the exact same mandatory dlopen-and-check treatment for it as it already has for
    // json_plugin above: opened by name (not open_all() — same reasoning as the json-only
    // comment above), then checked via OpenApiGeneratorRegistry::has_generator() below.
    utils::openapi::OpenApiGeneratorRegistry generator_registry;

    core::plugin::SharedLibrary plugin_store{"plugin"};
    plugin_store.scan("../../../build/plugins");
    auto json_open_res = plugin_store.open("../../../build/plugins/libjson_plugin.so");
    if (!json_open_res) {
        std::println(stderr, "build.cc: plugin load failed: {}",
                     json_open_res.error().get_message());
        return 1;
    }
    auto openapi_open_res = plugin_store.open("../../../build/plugins/libopenapi_generator.so");
    if (!openapi_open_res) {
        std::println(stderr, "build.cc: plugin load failed: {}",
                     openapi_open_res.error().get_message());
        return 1;
    }

    CongeladoHostCallbacks empty_host_cb{};
    auto build_res = plugin_store.build(empty_host_cb, {});
    if (!build_res) {
        std::println(stderr, "build.cc: plugin build failed: {}", build_res.error().get_message());
        return 1;
    }
    plugin_store.for_each([&format_registry, &generator_registry](
                              const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
        auto plugin = runtime->get_plugin();
        if (!plugin) {
            return;
        }
        if (auto format = congelado::heart::resolve_serde_format(*plugin)) {
            format_registry.add_format(std::move(format));
        }
        if (auto generator = congelado::heart::resolve_openapi_generator(*plugin)) {
            generator_registry.add_generator(std::move(generator));
        }
    });
    if (format_registry.find("application/json") == nullptr) {
        std::println(stderr, "build.cc: no JSON format plugin loaded — was json_plugin built?");
        return 1;
    }
    if (!generator_registry.has_generator()) {
        std::println(stderr,
                     "build.cc: no OpenAPI generator plugin loaded — was openapi_generator built?");
        return 1;
    }
    auto *doc_generator = generator_registry.get_generators().front().get();

    core::router::RouterContext<> engine_router;
    engine::EngineContext ctx;
    engine::register_routes(engine_router, ctx);

    std::filesystem::path openapi_path{"../generated/engine/openapi.json"};
    std::filesystem::path client_dir{"../generated/engine/client"};
    std::filesystem::create_directories(client_dir);

    if (auto write_res = doc_generator->write_document("Congelado Engine API", "1.0.0", openapi_path);
        !write_res) {
        std::println(stderr, "build.cc: failed to write '{}': {}", openapi_path.string(),
                     write_res.error());
        return 1;
    }

    auto client_result =
        doc_generator->generate_client_sdk(openapi_path, client_dir, "congelado_api", std::nullopt);
    if (!client_result) {
        std::println(stderr, "build.cc: client codegen failed: {}", client_result.error());
        return 1;
    }

    std::println("build.cc: wrote '{}' and generated client SDK in '{}'", openapi_path.string(),
                 client_dir.string());
    return 0;
}

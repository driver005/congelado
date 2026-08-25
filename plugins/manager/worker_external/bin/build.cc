#include <congelado/abi.h>
#include <cstdio>

import std;
import core_router;
import core_plugin;
import serde;
import congelado_heart;
import worker_runtime;
import utils_openapi;

// Build-time-only tool: generates the worker's own OpenAPI document + typed client SDK
// (congelado_worker_api::*) from worker_runtime::register_routes(). Same idiom as the engine's
// src/build.cc, applied to the worker runtime that now lives in this plugin. Registers the worker's
// routes into a throwaway RouterContext (which, via ApiRoute/ApiRouter's constructors, populates
// utils::openapi::Registry as a side effect), then writes the doc and feeds it into
// IOpenApiGenerator::generate_client_sdk().
//
// Run by xmake's build_tool() with curdir pinned to this file's own directory
// (plugins/manager/worker_external/bin), so paths below are relative to that: "../generated/worker"
// is plugins/manager/worker_external/generated/worker, and "../../../../build/plugins" is the
// repo-root build/plugins output dir.
int main() {
    // Force-load the JSON format + OpenAPI generator plugins by name (not open_all() — some plugins
    // can fail to dlopen in a given environment, and this tool only needs these two).
    serde::SerdeFormatRegistry format_registry;
    serde::SerdeFormatRegistry::set_active(&format_registry);

    utils::openapi::OpenApiGeneratorRegistry generator_registry;

    core::plugin::SharedLibrary plugin_store{"plugin"};
    plugin_store.scan("../../../../build/plugins");
    auto json_open_res = plugin_store.open("../../../../build/plugins/libjson_plugin.so");
    if (!json_open_res) {
        std::println(stderr, "build.cc: plugin load failed: {}",
                     json_open_res.error().get_message());
        return 1;
    }
    auto openapi_open_res = plugin_store.open("../../../../build/plugins/libopenapi_generator.so");
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

    core::router::RouterContext<> worker_router;
    worker::register_routes(worker_router);

    std::filesystem::path openapi_path{"../generated/worker/openapi.json"};
    std::filesystem::path client_dir{"../generated/worker/client"};
    std::filesystem::create_directories(client_dir);

    if (auto write_res = doc_generator->write_document("Congelado Worker API", "1.0.0", openapi_path);
        !write_res) {
        std::println(stderr, "build.cc: failed to write '{}': {}", openapi_path.string(),
                     write_res.error());
        return 1;
    }

    auto client_result = doc_generator->generate_client_sdk(openapi_path, client_dir,
                                                             "congelado_worker_api", std::nullopt);
    if (!client_result) {
        std::println(stderr, "build.cc: client codegen failed: {}", client_result.error());
        return 1;
    }

    std::println("build.cc: wrote '{}' and generated client SDK in '{}'", openapi_path.string(),
                 client_dir.string());
    return 0;
}

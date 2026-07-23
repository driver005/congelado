#include <cstdio>

import std;
import core_router;
import engine;
import worker;
import utils_openapi;
import congelado_client;

// Build-time-only tool: registers both the engine's and the worker's routes into their own
// throwaway RouterContexts (which, via ApiRoute/ApiRouter's constructors, populates
// utils::openapi::Registry as a side effect — see engine::register_routes'/
// worker::register_routes' own doc comments), then writes ONE combined OpenAPI document (both
// route sets land in the same process-global Registry, so a single Generator::generate() call
// already covers both) and feeds it into congelado::client::Generator to produce a single
// typed client SDK. Never linked into congelado_lib, the engine plugin, or the congelado_worker
// binary itself — xmake's generic build.cc feature runs this (no args — takes none) before
// either of those targets' real sources compile.
//
// Output lands inside plugins/engine/generated/ (not build/) — deliberately inside
// congelado_lib's own plugins/**.cppm glob, so a subsequent congelado_lib build picks the
// generated client module up and compiles it for real, rather than leaving it as dead,
// never-built output. Paths are relative to the project root, which is where xmake always
// runs build.cc tools from.
int main() {
    std::filesystem::path openapi_path{"plugins/engine/generated/openapi.json"};
    std::filesystem::path client_dir{"plugins/engine/generated/client"};
    std::filesystem::create_directories(client_dir);

    core::router::RouterContext<> engine_router;
    engine::EngineContext ctx;
    engine::register_routes(engine_router, ctx);

    core::router::RouterContext<> worker_router;
    worker::register_routes(worker_router);

    auto generator = utils::openapi::Generator{}
                          .title("Congelado API")
                          .version("1.0.0")
                          .output_path(openapi_path);

    if (auto write_res = generator.write(generator.generate()); !write_res) {
        std::println(stderr, "build.cc: failed to write '{}': {}", openapi_path.string(),
                     write_res.error());
        return 1;
    }

    auto client_result = congelado::client::Generator{}
                              .namespace_name("congelado_api")
                              .generate(openapi_path.string(), client_dir.string());
    if (!client_result) {
        std::println(stderr, "build.cc: client codegen failed: {}", client_result.error());
        return 1;
    }

    std::println("build.cc: wrote '{}' and generated client SDK in '{}'", openapi_path.string(),
                 client_dir.string());
    return 0;
}

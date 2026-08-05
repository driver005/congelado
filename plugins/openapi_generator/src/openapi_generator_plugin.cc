module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module openapi_generator_plugin;

import congelado_plugin;
import interfaces;
import std;
import core_router;
import utils_openapi;
import core_events;
import core_logger;

import :document;
import :schema_model;
import :dto_writer;
import :route_writer;
import :generator;
import :doc_generator;

// Runtime-loadable replacement for the two build-time-only codegen tools this used to be:
// utils::openapi::Generator (server-side OpenAPI document assembly/serialization/live-serve,
// moved in from include/utils/openapi/generator.cppm as :doc_generator) and
// congelado::client::Generator (the typed client SDK pipeline, moved in from
// sdk/client/{document,schema_model,dto_writer,route_writer,generator}.cppm) — both now sit
// behind interfaces::IOpenApiGenerator, discovered via the plugin ABI like json_plugin/http2/
// toml_plugin, instead of being directly `import`ed by build.cc/app.cppm/worker_main.cc.
class OpenApiGeneratorPlugin : public congelado::Plugin, public interfaces::IOpenApiGenerator {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"openapi_generator"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "openapi_generator"; }
    /**
     * @brief Version string for this build of the OpenAPI generator plugin.
     * @return `"0.1.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    /**
     * @brief Flags this as an OpenAPI-generator-capable plugin, so the host wires `openapi_get`
     * into the `_cap_dispatch` routing.
     * @return `CONGELADO_CAP_OPENAPI`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_OPENAPI;
    }

    /**
     * @brief Declares that protocol plugins (http2 etc.) must load after this one, so `/openapi`
     * is already registered by the time a protocol plugin compiles its route trie — same
     * ordering constraint the engine plugin declares for its own routes.
     * @return a span containing `"protocol"`.
     */
    [[nodiscard]] std::span<const std::string_view>
    get_load_before_types() const noexcept override {
        static constexpr std::string_view TYPES[] = {"protocol"};
        return TYPES;
    }

    /**
     * @brief Self-registers the live `/openapi` route onto the host's router — this plugin owns
     * that one route entirely now; the host just dlopens it and lets on_load wire it up, same as
     * every other route-registering plugin (see `engine.cc`'s own on_load for the identical
     * pattern). Previously the host manually called `serve_document()` and `add_route()`d the
     * result itself; that's now internal to this plugin.
     * @note No router context, no motion — logs and bails instead of dereferencing a null
     * pointer, same defensive shape `engine.cc`'s on_load uses.
     * @param host the host callback table; used here to fetch the router context.
     * @param cfg optional per-process `title`/`version` override (a `[plugins.openapi_generator]`
     * table in that process's own config file) — defaults to `"Congelado API"`/`"1.0.0"` if unset.
     */
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) override {
        auto *router_ctx = congelado::router_ctx<core::router::RouterContext<>>(host);
        if (router_ctx == nullptr) {
            core::logger::error("openapi_generator", "no router context");
            core::events::publish("openapi_generator.no_router_context");
            return;
        }

        std::string title = "Congelado API";
        if (auto val = congelado::config_get(cfg, "title")) {
            title = *val;
        }
        std::string version = "1.0.0";
        if (auto val = congelado::config_get(cfg, "version")) {
            version = *val;
        }

        router_ctx->add_route(serve_document(title, version));
        core::logger::important("openapi_generator", "serving live '{}' v{} document at /openapi",
                                title, version);
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IOpenApiGenerator` surface.
     * @return this instance, upcast to `interfaces::IOpenApiGenerator*`.
     */
    void *openapi_get() noexcept { return static_cast<interfaces::IOpenApiGenerator *>(this); }

    /// @brief Generator backend name reported through the capability interface. @return `"openapi_generator"`.
    [[nodiscard]] std::string_view generator_name() const noexcept override {
        return "openapi_generator";
    }

    /**
     * @brief Builds the OpenAPI document fresh from the process-wide route/schema registries and
     * writes it out as JSON — thin wrapper around the moved-in `utils::openapi::Generator`.
     * @param title the document's `info.title`.
     * @param version the document's `info.version`.
     * @param output_path the filesystem path to write the serialized document to.
     * @return nothing on success, or an error string if serialization/write failed.
     */
    [[nodiscard]] std::expected<void, std::string>
    write_document(std::string_view title, std::string_view version,
                   std::filesystem::path const &output_path) const override {
        core::logger::debug("openapi_generator", "writing '{}' v{} to {}", title, version,
                            output_path.string());
        auto generator = utils::openapi::Generator{}
                              .title(title)
                              .version(version)
                              .output_path(output_path);
        auto result = generator.write(generator.generate());
        if (!result) {
            core::logger::warning("openapi_generator", "write_document failed: {}", result.error());
            core::events::publish("openapi_generator.write_document_failed", {{"error", result.error()}});
        }
        return result;
    }

    /**
     * @brief Builds a live GET route serving the freshly-regenerated OpenAPI document on every
     * hit — thin wrapper around `utils::openapi::Generator::serve()`.
     * @param title the document's `info.title`.
     * @param version the document's `info.version`.
     * @return a route wired up to serve the live-generated OpenAPI JSON.
     */
    [[nodiscard]] core::router::Route<>
    serve_document(std::string_view title, std::string_view version) const override {
        // Logged once here, at route-build time — not inside the route's own handler, which
        // regenerates the document on every hit; a per-request debug line there would just spam
        // the log on every /openapi fetch for no diagnostic gain.
        core::logger::debug("openapi_generator", "serving live '{}' v{} document", title, version);
        return utils::openapi::Generator{}.title(title).version(version).serve();
    }

    /**
     * @brief Loads an OpenAPI document and generates a typed client SDK — thin wrapper around
     * the moved-in `congelado::client::Generator`.
     * @param openapi_path path to the OpenAPI document to load.
     * @param output_dir directory the generated `dto.cppm`/`routes.cppm` files get written into.
     * @param namespace_name base namespace the generated routes nest under.
     * @param shared_models name of an existing DTO module to import instead of generating a
     * fresh one, or empty to generate `"{namespace_name}_dto"` as usual.
     * @return nothing on success, or an error string describing whatever step failed.
     */
    [[nodiscard]] std::expected<void, std::string>
    generate_client_sdk(std::filesystem::path const &openapi_path,
                        std::filesystem::path const &output_dir,
                        std::string_view namespace_name,
                        std::optional<std::string_view> shared_models) const override {
        core::logger::debug("openapi_generator", "generating client SDK '{}' from {} into {}",
                            namespace_name, openapi_path.string(), output_dir.string());
        auto generator = congelado::client::Generator{}.namespace_name(namespace_name);
        if (shared_models) {
            generator = std::move(generator).shared_models(*shared_models);
        }
        auto result = std::move(generator).generate(openapi_path, output_dir);
        if (!result) {
            core::logger::warning("openapi_generator", "generate_client_sdk failed: {}",
                                  result.error());
            core::events::publish("openapi_generator.generate_client_sdk_failed",
                                  {{"error", result.error()}});
        }
        return result;
    }
};

CONGELADO_PLUGIN(OpenApiGeneratorPlugin);

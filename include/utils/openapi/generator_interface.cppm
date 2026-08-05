export module utils_openapi:generator_interface;

import std;
import core_router;

// The OpenAPI/client-SDK codegen surface as a genuine plugin capability, mirroring
// interfaces::ISerdeFormat's shape (include/interfaces/serde.cppm). This deliberately does NOT
// live in the `interfaces` module itself: `core::router::Route<>` crosses this boundary by value
// (see serve_document() below), and core_router already imports `interfaces` (core_router:handler
// needs interfaces::HandlerFn) — an `interfaces` partition importing core_router back would be a
// genuine module cycle, not just an inconvenience. `utils_openapi` already sits downstream of
// both `core_router` and `interfaces` with no such cycle (utils_openapi:route already imports
// both), so this partition lives here instead, under the same `interfaces` namespace as every
// other capability interface for naming consistency.
export namespace interfaces {

class IOpenApiGenerator {
  public:
    /**
     * @brief Virtual dtor, default's good — generator backends clean up fine through the base
     * pointer, no extra motion needed.
     */
    virtual ~IOpenApiGenerator() = default;
    IOpenApiGenerator() = default;
    IOpenApiGenerator(const IOpenApiGenerator &) = delete;
    IOpenApiGenerator &operator=(const IOpenApiGenerator &) = delete;
    IOpenApiGenerator(IOpenApiGenerator &&) = delete;
    IOpenApiGenerator &operator=(IOpenApiGenerator &&) = delete;

    /**
     * @brief A short human-readable name for this generator backend, for logs/diagnostics.
     * @return the generator's name (e.g. `"openapi_generator"`).
     */
    [[nodiscard]] virtual std::string_view generator_name() const noexcept = 0;

    /**
     * @brief Builds the OpenAPI document fresh from the process-wide route/schema registries and
     * writes it to `output_path` as JSON — mirrors `utils::openapi::Generator::generate()` +
     * `write()` chained together.
     * @param title the document's `info.title`.
     * @param version the document's `info.version`.
     * @param output_path the filesystem path to write the serialized document to.
     * @return nothing on success, or an error string if serialization/write failed.
     */
    [[nodiscard]] virtual std::expected<void, std::string>
    write_document(std::string_view title, std::string_view version,
                   std::filesystem::path const &output_path) const = 0;

    /**
     * @brief Builds a live GET route that regenerates the OpenAPI document from the registries
     * on every hit and serves it as JSON — mirrors `utils::openapi::Generator::serve()`.
     * @warning Same ordering requirement as the class this wraps: has to land in RouterContext
     * before the protocol plugin's on_load compiles the trie.
     * @param title the document's `info.title`.
     * @param version the document's `info.version`.
     * @return a route wired up to serve the live-generated OpenAPI JSON.
     */
    [[nodiscard]] virtual core::router::Route<>
    serve_document(std::string_view title, std::string_view version) const = 0;

    /**
     * @brief Loads an OpenAPI document from `openapi_path` and generates a typed client SDK
     * (`dto.cppm`/`routes.cppm`) into `output_dir` — mirrors
     * `congelado::client::Generator::namespace_name(...).shared_models(...).generate(...)`.
     * @param openapi_path path to the OpenAPI document to load.
     * @param output_dir directory the generated files get written into.
     * @param namespace_name base namespace the generated routes nest under.
     * @param shared_models name of an existing DTO module to import instead of generating a
     * fresh one, or empty to generate `"{namespace_name}_dto"` as usual.
     * @return nothing on success, or an error string describing whatever step failed.
     */
    [[nodiscard]] virtual std::expected<void, std::string>
    generate_client_sdk(std::filesystem::path const &openapi_path,
                        std::filesystem::path const &output_dir,
                        std::string_view namespace_name,
                        std::optional<std::string_view> shared_models) const = 0;
};

} // namespace interfaces

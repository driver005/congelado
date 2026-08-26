export module openapi_generator_plugin:generator;

import std;
import serde;
import core_generator;
import :document;
import :schema_model;
import :dto_writer;
import :route_writer;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace congelado::client {

class Generator
{
public:
    /** @brief Spins up a Generator with the defaults — "client" namespace, no shared DTO
     * module. */
    Generator() = default;

    /**
     * @brief Fluent setter for the base namespace generated routes nest under — bet, chain it
     * right into the next builder call.
     * @param value namespace name to use.
     * @return this Generator, moved, for chaining.
     */
    Generator namespace_name(std::string_view value) &&
    {
        m_namespace = std::string{value};
        return std::move(*this);
    }

    /**
     * @brief Fluent setter that points the generator at an already-existing DTO module instead
     * of generating one — big W when you don't want duplicate DTOs across multiple client SDKs.
     * @param moduleName name of the pre-existing module containing the DTO types.
     * @return this Generator, moved, for chaining.
     */
    Generator shared_models(std::string_view moduleName) &&
    {
        m_shared_models_module = std::string{moduleName};
        return std::move(*this);
    }

    /**
     * @brief The main entry point — loads an OpenAPI document, parses its schemas and
     * operations, then writes out `dto.cppm` (unless a shared models module was configured) and
     * `routes.cppm` into `output_dir`. This is the whole pipeline in one call, no cap.
     * @param openapi_path path to the OpenAPI document to load.
     * @param output_dir directory the generated `dto.cppm`/`routes.cppm` files get written
     * into.
     * @return nothing on success, or an error string describing whatever step failed (document
     * load, schema/operation parsing, DTO/route generation, or file write).
     */
    [[nodiscard]] std::expected<void, std::string> generate(
        const std::filesystem::path& openapi_path, const std::filesystem::path& output_dir
    ) const
    {
        // Load the OpenAPI document first — nothing downstream can happen without it.
        auto document = Document::load(openapi_path);
        if (!document) {
            return std::unexpected{document.error()};
        }

        // Parse both schemas and operations off the loaded document before generating anything.
        auto schemas = parse_components(*document);
        if (!schemas) {
            return std::unexpected{schemas.error()};
        }
        auto operations = parse_operations(*document);
        if (!operations) {
            return std::unexpected{operations.error()};
        }

        std::string dto_module_name =
            m_shared_models_module.value_or(std::format("{}_dto", m_namespace));

        // Only generate dto.cppm when there's no pre-existing shared models module to point at
        // instead — otherwise we'd lowkey be duplicating DTOs that already exist elsewhere.
        if (!m_shared_models_module) {
            auto dto_source = DtoWriter::write(*schemas, dto_module_name);
            if (!dto_source) {
                return std::unexpected{dto_source.error()};
            }
            auto dto_path = output_dir / "dto.cppm";
            if (auto result = core::generator::Generator::write(dto_path, *dto_source); !result) {
                return std::unexpected{result.error()};
            }
        }

        // routes.cppm always gets generated, whether the DTOs came from this run or a shared
        // module.
        auto routes_module_name = std::format("{}_routes", m_namespace);
        auto routes_source =
            RouteWriter::write(*operations, routes_module_name, dto_module_name, m_namespace);
        if (!routes_source) {
            return std::unexpected{routes_source.error()};
        }
        auto routes_path = output_dir / "routes.cppm";
        if (auto result = core::generator::Generator::write(routes_path, *routes_source); !result) {
            return std::unexpected{result.error()};
        }

        return {};
    }

private:
    /**
     * @brief Pulls every named schema out of `components.schemas` and parses each one into a
     * SchemaType. A document with no `components.schemas` at all? Still a W — that's a valid,
     * schema-less case, not an error.
     * @param document the loaded OpenAPI document.
     * @return every named schema, keyed by name, or an error if a schema fails to parse.
     */
    [[nodiscard]] static std::expected<std::unordered_map<std::string, SchemaType>, std::string>
    parse_components(const serde::Value& document)
    {
        std::unordered_map<std::string, SchemaType> schemas;
        auto named = Document::at(document, {"components", "schemas"});
        if (!named) {
            return schemas; // no components — valid, just empty
        }
        auto object = named->to_object();
        if (!object) {
            return schemas;
        }
        // Parse every named schema — first failure bails the whole call out with a
        // schema-qualified error so it's obvious which one's cooked.
        for (auto& [name, value]: *object) {
            SchemaType schema;
            if (auto result = schema.parse(value); !result) {
                return std::unexpected{std::format("schema '{}': {}", name, result.error())};
            }
            schemas.emplace(name, std::move(schema));
        }
        return schemas;
    }

    /**
     * @brief Scans an operation's `responses` object for the first 2xx status and, if it
     * carries a JSON schema, parses and attaches it to `operation` — only one response shape
     * rides per operation, so the first 2xx found wins and the scan stops there.
     * @param operation the operation being built; mutated in place on a match.
     * @param operation_element the raw OpenAPI operation object (`paths.<path>.<method>`).
     * @return nothing on success (including "no 2xx response" — that's not an error), or an
     * error if the matched response's schema fails to parse.
     */
    [[nodiscard]] static std::expected<void, std::string>
    parse_operation_response(OperationInfo& operation, const serde::Value& operation_element)
    {
        auto responses_value = Document::at(operation_element, {"responses"});
        if (!responses_value) {
            return {};
        }
        auto responses = responses_value->to_object();
        if (!responses) {
            return {};
        }
        for (auto& [status, response_element]: *responses) {
            if (status.empty() || status.front() != '2') {
                continue;
            }
            if (auto response_schema =
                    Document::at(response_element, {"content", "application/json", "schema"})) {
                SchemaType schema;
                if (auto result = schema.parse(*response_schema); !result) {
                    return std::unexpected{result.error()};
                }
                operation.set_response(std::move(schema));
            }
            break;
        }
        return {};
    }

    /**
     * @brief Builds one OperationInfo for a single path+method pair — grabs the JSON request
     * body schema (if any) via parse_operation_response()'s sibling inline check, then
     * delegates the response side to parse_operation_response().
     * @param path the OpenAPI path this operation lives under.
     * @param method the HTTP method this operation responds to.
     * @param operation_element the raw OpenAPI operation object (`paths.<path>.<method>`).
     * @return the parsed operation, or an error if the request/response schema fails to parse.
     */
    [[nodiscard]] static std::expected<OperationInfo, std::string> parse_operation(
        const std::string& path, const std::string& method, const serde::Value& operation_element
    )
    {
        OperationInfo operation;
        operation.set_path(path);
        operation.set_method(method);

        // Request body schema is optional — only grab it if the JSON content type actually
        // shows up under requestBody.
        if (auto request_schema = Document::at(
                operation_element, {"requestBody", "content", "application/json", "schema"}
            )) {
            SchemaType schema;
            if (auto result = schema.parse(*request_schema); !result) {
                return std::unexpected{result.error()};
            }
            operation.set_request_body(std::move(schema));
        }

        if (auto result = parse_operation_response(operation, operation_element); !result) {
            return std::unexpected{result.error()};
        }

        return operation;
    }

    /**
     * @brief Walks `paths` in the OpenAPI document and builds one OperationInfo per path+method
     * pair via parse_operation().
     * @param document the loaded OpenAPI document.
     * @return every parsed operation, or an error if `paths` is missing/malformed or a request/
     * response schema fails to parse.
     */
    [[nodiscard]] static std::expected<std::vector<OperationInfo>, std::string>
    parse_operations(const serde::Value& document)
    {
        std::vector<OperationInfo> operations;
        // "paths" is mandatory and must be an object — no fallback here, this is the whole
        // point of an OpenAPI doc.
        auto paths_value = Document::at(document, {"paths"});
        if (!paths_value) {
            return std::unexpected{"document missing 'paths'"};
        }
        auto paths = paths_value->to_object();
        if (!paths) {
            return std::unexpected{"'paths' must be an object"};
        }
        for (auto& [path, methods_element]: *paths) {
            auto methods = methods_element.to_object();
            if (!methods) {
                continue;
            }
            // One OperationInfo per path+method pair.
            for (auto& [method, operation_element]: *methods) {
                auto op = parse_operation(path, method, operation_element);
                if (!op) {
                    return std::unexpected{op.error()};
                }
                operations.push_back(std::move(*op));
            }
        }
        return operations;
    }

    std::string m_namespace{"client"};
    std::optional<std::string> m_shared_models_module;
};

} // namespace congelado::client

#ifdef CONGELADO_TEST
// Test-only note: Generator's only entry point is generate(const filesystem::path&, ...), and
// its very first step is Document::load() -> Document::parse() -> serde::Ser::decode_generic(),
// which (see document.cppm's own tests, same isolated test target) always fails here with "no
// format plugin loaded for 'application/json'" -- no JSON format plugin is linked into this
// target. Unlike schema_model.cppm/document.cppm, Generator has no way to hand it an
// already-built serde::Value directly (its public API only takes a filesystem::path), so every
// private helper below Document::load in the pipeline -- parse_components(),
// parse_operation_response(), parse_operation(), parse_operations() -- is genuinely unreachable
// from this test target; there's no seam to exercise them through. What's left testable here is
// generate()'s two pre-parse failure branches (missing file / no format plugin) and that the
// namespace_name()/shared_models() fluent builder chain doesn't misbehave (double-move, etc.).
namespace openapi_gen_client_generator_tests {
using namespace boost::ut;
using congelado::client::Generator;

suite<"Generator"> client_generator_suite = [] {
    "generate(): a nonexistent openapi document path fails to open"_test = [] {
        Generator generator;

        auto result = generator.generate("/nonexistent/path/does/not/exist.json", "/tmp");

        expect(not result.has_value()) << fatal;
        expect(result.error().contains("failed to open"));
    };

    "generate(): an existing file still fails at the parse step (no format plugin loaded)"_test =
        [] {
            auto path =
                std::filesystem::temp_directory_path() / "congelado_client_generator_test.json";
            {
                std::ofstream out{path};
                out << R"({"paths": {}})";
            }
            Generator generator;

            auto result = generator.generate(path, std::filesystem::temp_directory_path());

            expect(not result.has_value()) << fatal;
            expect(result.error().contains("failed to parse"));
            expect(result.error().contains("no format plugin loaded for 'application/json'"));

            std::filesystem::remove(path);
        };

    "namespace_name()/shared_models() fluent chain builds without crashing or double-moving"_test =
        [] {
            auto generator = Generator{}.namespace_name("my_client").shared_models("shared_dto");

            // Still fails the same way -- generate() bails at Document::load() before ever
            // touching m_namespace/m_shared_models_module -- this just confirms the chained,
            // moved-through Generator is otherwise usable (no crash, no UB from the moves).
            auto result = generator.generate("/nonexistent/path/does/not/exist.json", "/tmp");

            expect(not result.has_value()) << fatal;
            expect(result.error().contains("failed to open"));
        };

    "namespace_name() alone (no shared_models) also builds and behaves the same way"_test = [] {
        auto generator = Generator{}.namespace_name("solo_client");

        auto result = generator.generate("/nonexistent/path/does/not/exist.json", "/tmp");

        expect(not result.has_value()) << fatal;
        expect(result.error().contains("failed to open"));
    };
};

} // namespace openapi_gen_client_generator_tests
#endif

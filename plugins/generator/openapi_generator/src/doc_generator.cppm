export module openapi_generator_plugin:doc_generator;

import std;
import interfaces;
import core_router;
import core_generator;
import serde;
import utils_openapi;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::openapi {

class Generator {
  public:
    /**
     * @brief Default ctor — blank generator, default info/output-path/serve-path, fill in via
     * the builder chain below. Clean slate, no motion yet.
     */
    Generator() = default;

    /**
     * @brief Builder chain — sets the document's title.
     * @param value the title text.
     * @return `*this`, moved, so the chain keeps going.
     */
    Generator title(std::string_view value) && {
        m_info.set_title(std::string{value});
        return std::move(*this);
    }
    /**
     * @brief Builder chain — sets the document's version string.
     * @param value the version text.
     * @return `*this`, moved, so the chain keeps going.
     */
    Generator version(std::string_view value) && {
        m_info.set_version(std::string{value});
        return std::move(*this);
    }
    /**
     * @brief Builder chain — sets where write() drops the generated JSON file.
     * @param value the output filesystem path.
     * @return `*this`, moved, so the chain keeps going.
     */
    Generator output_path(std::filesystem::path value) && {
        m_output_path = std::move(value);
        return std::move(*this);
    }
    /**
     * @brief Builder chain — sets which path serve() mounts the live GET handler on.
     * @param value the serve path.
     * @return `*this`, moved, so the chain keeps going.
     */
    Generator serve_path(std::string_view value) && {
        m_serve_path = std::string{value};
        return std::move(*this);
    }

    /**
     * @brief Builds the full OpenAPI Document fresh from Registry — walks every registered
     * route, skips the ones with zero operations (pure path-only nodes with no method
     * handlers), resolves each route's full path by walking its base_router chain, and pulls
     * every collected schema out of SchemaRegistry into components.schemas. No cap, no caching
     * here — cheap enough to just redo on demand (see serve() below for why that matters).
     * @return the freshly assembled OpenAPI document.
     */
    [[nodiscard]] Document generate() const {
        Document document;
        document.set_info(m_info);

        // Walk every registered route — pure path-only nodes with zero operations get
        // skipped, no cap, no point emitting an empty paths entry for those.
        const auto &routes = Registry::get_routes();
        for (const auto &route : routes) {
            if (route.get_operations().empty()) {
                continue;
            }

            // Resolve this route's full path once, then register every method it
            // declares under that same path.
            auto full_path = resolve_full_path(route, routes);
            for (const auto &[method, operation] : route.get_operations()) {
                auto method_name = to_lower(interfaces::io::types::method_str(
                    static_cast<interfaces::io::types::Method>(method)));
                document.add_operation(full_path, method_name, operation);
            }
        }

        // Pull every schema build_schema<T>() has collected into SchemaRegistry over the
        // process's lifetime — that's the whole components.schemas payload, bet.
        Components components;
        for (const auto &[name, schema] : SchemaRegistry::getSchemas()) {
            components.add_schema(name, schema);
        }
        document.set_components(std::move(components));

        return document;
    }

    /**
     * @brief Serializes `document` to JSON and writes it out to m_output_path. Straightforward
     * motion, bet.
     * @param document the document to write.
     * @return nothing on success, or an error string if the write failed.
     */
    [[nodiscard]] std::expected<void, std::string> write(const Document &document) const {
        auto encoded = serde::Ser::serialize("application/json", document);
        std::string text(reinterpret_cast<const char *>(encoded.data()), encoded.size());  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) — byte-vector-to-string is the standard shape Ser::serialize's callers use to get text back out
        return core::generator::Generator::write(m_output_path, text);
    }

    // Registers a GET handler that builds the document fresh from Registry on every
    // request (cheap: a few dozen routes at most). This route must be registered into
    // RouterContext *before* the protocol plugin's on_load compiles the trie (protocol
    // plugins do this eagerly, inside their own on_load — not deferred), so it can't
    // simply capture a Document generated after all plugins have loaded.
    /**
     * @brief Builds a GET route that regenerates the document from Registry on every hit and
     * serves it as JSON — no caching here either, same deal as generate(), and with only a
     * handful of routes tops that's a total non-issue performance-wise. Not cooked, don't
     * worry about it.
     * @warning Has to land in RouterContext before the protocol plugin's on_load compiles the
     * trie (protocol plugins do that eagerly inside their own on_load, not deferred) — see the
     * comment right above this method for the full rundown on why a pre-baked Document snapshot
     * won't cut it here.
     * @return a route wired up to serve the live-generated OpenAPI JSON.
     */
    [[nodiscard]] core::router::Route<> serve() const {
        Info info = m_info;
        return core::router::Route<>{m_serve_path}.get(
            [info](interfaces::io::IRequest & /*req*/, interfaces::io::IResponse &res,
                   std::function<void()> send) {
                // Rebuild the document fresh off the captured info and serialize it to
                // JSON bytes — regenerated on every hit, see generate()'s doc comment for
                // why that's a non-issue perf-wise.
                Generator generator;
                generator.m_info = info;
                auto bytes = serde::Ser::serialize("application/json", generator.generate());
                // Then write the response: content type, body, status, in that order.
                res.add_content_type("application/json");
                res.add_body(bytes);
                res.add_status(interfaces::io::types::Status::OK);
                send();
            });
    }

  private:
    /**
     * @brief Lowercases a string, ASCII-only — used to turn "GET"/"POST" method names into the
     * lowercase keys the OpenAPI "paths" map expects.
     * @param value the string to lowercase.
     * @return the lowercased copy.
     */
    [[nodiscard]] static std::string to_lower(std::string_view value) {
        std::string result{value};
        std::ranges::transform(result, result.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return result;
    }

    // OpenAPI paths use "{name}" for path parameters; this router's convention uses ":name".
    // A route's own path segment can itself embed a literal '/' (e.g. ":name/enqueue" is one
    // segment, not two), so each '/'-delimited part is converted independently.
    /**
     * @brief Converts this router's ":name"-style path params into OpenAPI's "{name}" style,
     * segment by segment — see the comment right above for why it can't just blindly split
     * the whole path on '/'.
     * @param segment one raw path segment (which can itself contain embedded slashes).
     * @return the segment with any ":param" pieces rewritten to "{param}".
     */
    [[nodiscard]] static std::string to_openapi_path(std::string_view segment) {
        std::string result;
        std::size_t search_start = 0;
        while (true) {
            // Peel off the next '/'-delimited part, whatever's left after search_start.
            auto slash_pos = segment.find('/', search_start);
            auto path_segment = segment.substr(
                search_start,
                slash_pos == std::string_view::npos ? std::string_view::npos
                                                    : slash_pos - search_start);
            // Rejoin with '/' as we go — skip it on the very first part so there's no
            // stray leading slash.
            if (!result.empty()) {
                result += '/';
            }
            // ":name" becomes "{name}"; everything else rides through untouched.
            if (!path_segment.empty() && path_segment.front() == ':') {
                result += std::format("{{{}}}", path_segment.substr(1));
            } else {
                result += std::string{path_segment};
            }
            // No more '/' left means this was the last part — done.
            if (slash_pos == std::string_view::npos) {
                break;
            }
            search_start = slash_pos + 1;
        }
        return result;
    }

    // Walks the base_router/router_number parent chain the same way
    // core::router::RouterContext::build() does (builder.cppm:142-238), but to reconstruct a
    // full path string instead of a compiled trie node.
    /**
     * @brief Reconstructs a route's full path by walking the base_router/router_number parent
     * chain — same walk core::router::RouterContext::build() does when compiling the trie,
     * except this rebuilds a plain path string instead of a trie node (see the comment above
     * for the exact line reference it mirrors). Same motion, different output shape, bet.
     * @param route the route whose full path is being resolved.
     * @param all_routes every registered route, needed to look up each ancestor by router
     * number.
     * @return the fully resolved path (e.g. "/tasks/{id}/enqueue"), or "/" if it resolves
     * empty.
     */
    [[nodiscard]] static std::string resolve_full_path(const RouteMeta &route,
                                                     const std::vector<RouteMeta> &all_routes) {
        // Seed with the route's own segment first (if it has one — a pure router node
        // might not).
        std::vector<std::string> segments;
        if (!route.get_path().empty()) {
            segments.push_back(to_openapi_path(route.get_path()));
        }

        // Then climb the base_router chain, collecting each ancestor's own segment, bet —
        // bails clean if an ancestor's gone missing from all_routes instead of looping
        // forever.
        std::size_t base_router_number = route.get_base_router();
        while (base_router_number != 0) {
            auto route_iter = std::ranges::find_if(all_routes, [&](const auto &candidate) {
                return candidate.get_router_number() == base_router_number;
            });
            if (route_iter == all_routes.end()) {
                break;
            }
            if (!route_iter->get_path().empty()) {
                segments.push_back(to_openapi_path(route_iter->get_path()));
            }
            base_router_number = route_iter->get_base_router();
        }

        // Segments got collected child-to-root, so reverse to root-to-child and stitch
        // with '/' — an empty result (no segments anywhere) falls back to bare "/".
        std::string result;
        for (const auto &segment : segments | std::views::reverse) {
            result += '/';
            result += segment;
        }
        return result.empty() ? "/" : result;
    }

    Info m_info;
    std::filesystem::path m_output_path{"openapi.json"};
    std::string m_serve_path{"/openapi"};
};

} // namespace utils::openapi

#ifdef CONGELADO_TEST
// Test-only note: utils::openapi::Registry and utils::openapi::SchemaRegistry (both defined
// in include/utils/openapi/{registry,schema}.cppm, outside this pass's 4-file scope) are
// process-wide, append-only singletons shared across every test in this binary -- same
// caveat those files' own tests already document: assertions here are relative
// (before/after, or checking a specific distinctively-named/numbered entry we just added),
// never assuming either registry starts empty. Router numbers and path/schema names below
// are all namespaced with a "docgen_test_"/9100xx prefix to keep this suite's fixtures from
// colliding with anything else registered in the same process.
namespace openapi_gen_doc_generator_tests {
using namespace boost::ut;
using utils::openapi::Document;
using utils::openapi::Generator;
using utils::openapi::Operation;
using utils::openapi::RouteMeta;
using utils::openapi::SchemaObject;
using utils::openapi::SchemaRegistry;

suite<"utils::openapi::Generator"> doc_generator_suite = [] {
    "generate(): a route with zero registered operations is skipped entirely"_test = [] {
        RouteMeta meta;
        meta.set_path("docgen_test_no_ops");
        meta.set_router_number(910001);
        meta.set_base_router(0);
        utils::openapi::Registry::add_route(meta);

        auto document = Generator{}.generate();

        expect(not document.get_paths().contains("/docgen_test_no_ops"));
    };

    "generate(): a root route (no ancestors) resolves its own path, ':param' segments become "
    "'{param}', and an embedded '/' inside one segment stays part of that segment"_test = [] {
        RouteMeta meta;
        meta.set_path(":taskId/enqueue");
        meta.set_router_number(910002);
        meta.set_base_router(0);
        Operation operation;
        operation.set_summary("docgen_test_enqueue");
        meta.add_operation(static_cast<std::uint8_t>(interfaces::io::types::Method::GET),
                           std::move(operation));
        utils::openapi::Registry::add_route(meta);

        auto document = Generator{}.generate();

        expect(document.get_paths().contains("/{taskId}/enqueue")) << fatal;
        expect(document.get_paths().at("/{taskId}/enqueue").contains("get")) << fatal;
        expect(document.get_paths().at("/{taskId}/enqueue").at("get").get_summary() ==
              "docgen_test_enqueue");
    };

    "generate(): walks the base_router parent chain, stitching ancestor segments root-to-child"_test =
        [] {
        RouteMeta root;
        root.set_path("docgen_test_tasks");
        root.set_router_number(910010);
        root.set_base_router(0);
        utils::openapi::Registry::add_route(root);

        RouteMeta child;
        child.set_path(":id");
        child.set_router_number(910011);
        child.set_base_router(910010);
        utils::openapi::Registry::add_route(child);

        RouteMeta leaf;
        leaf.set_path("comments");
        leaf.set_router_number(910012);
        leaf.set_base_router(910011);
        Operation operation;
        operation.set_summary("docgen_test_comments");
        leaf.add_operation(static_cast<std::uint8_t>(interfaces::io::types::Method::POST),
                           std::move(operation));
        utils::openapi::Registry::add_route(leaf);

        auto document = Generator{}.generate();

        auto path = "/docgen_test_tasks/{id}/comments";
        expect(document.get_paths().contains(path)) << fatal;
        expect(document.get_paths().at(path).contains("post")) << fatal;
        expect(document.get_paths().at(path).at("post").get_summary() == "docgen_test_comments");
    };

    "generate(): a route resolving to no segments anywhere falls back to the bare '/' path"_test =
        [] {
        RouteMeta meta;
        // No set_path() call -- own segment stays empty -- and no ancestors either.
        meta.set_router_number(910020);
        meta.set_base_router(0);
        Operation operation;
        operation.set_summary("docgen_test_root_fallback");
        meta.add_operation(static_cast<std::uint8_t>(interfaces::io::types::Method::PATCH),
                           std::move(operation));
        utils::openapi::Registry::add_route(meta);

        auto document = Generator{}.generate();

        expect(document.get_paths().contains("/")) << fatal;
        expect(document.get_paths().at("/").contains("patch")) << fatal;
        expect(document.get_paths().at("/").at("patch").get_summary() ==
              "docgen_test_root_fallback");
    };

    "generate(): every registered method on a route lowercases into its own paths[...][method] key"_test =
        [] {
        RouteMeta meta;
        meta.set_path("docgen_test_multi");
        meta.set_router_number(910030);
        meta.set_base_router(0);
        Operation list_op;
        list_op.set_summary("docgen_test_list");
        meta.add_operation(static_cast<std::uint8_t>(interfaces::io::types::Method::GET),
                           std::move(list_op));
        Operation create_op;
        create_op.set_summary("docgen_test_create");
        meta.add_operation(static_cast<std::uint8_t>(interfaces::io::types::Method::POST),
                           std::move(create_op));
        utils::openapi::Registry::add_route(meta);

        auto document = Generator{}.generate();

        auto path = "/docgen_test_multi";
        expect(document.get_paths().contains(path)) << fatal;
        expect(document.get_paths().at(path).at("get").get_summary() == "docgen_test_list");
        expect(document.get_paths().at(path).at("post").get_summary() == "docgen_test_create");
    };

    "generate(): every schema registered in SchemaRegistry flows into components.schemas"_test =
        [] {
        SchemaObject schema;
        schema.set_type("string");
        SchemaRegistry::addSchema("DocGenTestSchema", schema);

        auto document = Generator{}.generate();

        expect(document.get_components().get_schemas().contains("DocGenTestSchema")) << fatal;
        expect(document.get_components().get_schemas().at("DocGenTestSchema").get_type() ==
              "string");
    };

    "generate(): title()/version() flow from the builder chain into the generated document's info"_test =
        [] {
        auto document = Generator{}.title("Docgen Test API").version("9.9.9").generate();

        expect(document.get_info().get_title() == "Docgen Test API");
        expect(document.get_info().get_version() == "9.9.9");
    };

    "write(): serializes the document and writes it out at output_path()"_test = [] {
        auto path =
            std::filesystem::temp_directory_path() / "congelado_doc_generator_test_write.json";
        auto generator = Generator{}.output_path(path);

        auto result = generator.write(Document{});

        expect(result.has_value()) << fatal;
        std::ifstream in{path};
        std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
        // No JSON format plugin is linked in this isolated test target (see document.cppm's
        // own tests for the same property) -- serde::Ser::serialize() never fails outright
        // though, it just falls back to this fixed error payload, which write() still
        // faithfully writes to disk.
        expect(content == R"({"error":"no format plugin loaded for 'application/json'"})");

        std::filesystem::remove(path);
    };

    "write(): fails cleanly when output_path()'s directory doesn't exist"_test = [] {
        auto generator =
            Generator{}.output_path("/nonexistent_dir_xyz_doc_gen/out.json");

        auto result = generator.write(Document{});

        expect(not result.has_value()) << fatal;
        expect(result.error().contains("failed to write"));
    };

    "serve(): builds a Route with a GET handler registered at the configured serve_path"_test =
        [] {
        auto route = Generator{}.serve_path("/docgen_test_serve").title("X").version("1").serve();

        expect(route.get_path() == "docgen_test_serve");
        expect(route.get_handlers().find(interfaces::io::types::Method::GET) != nullptr);
        expect(route.get_handlers().find(interfaces::io::types::Method::POST) == nullptr);
    };

    "serve(): defaults to the '/openapi' serve path when serve_path() is never called"_test = [] {
        auto route = Generator{}.serve();

        expect(route.get_path() == "openapi");
    };
};

} // namespace openapi_gen_doc_generator_tests
#endif

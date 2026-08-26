export module openapi_generator_plugin:route_writer;

import std;
import core_generator;
import :schema_model;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace congelado::client {

class OperationInfo
{
public:
    /** @brief Builds an empty operation — no path/method/body/response set yet, blank canvas.
     */
    OperationInfo() = default;

    /**
     * @brief Locks in the URL path template (e.g. "/api/v1/tasks/{name}").
     * @param value path string.
     */
    void set_path(std::string value)
    {
        m_path = std::move(value);
    }

    /** @brief Locks in the HTTP method (e.g. "get", "post"). @param value method string. */
    void set_method(std::string value)
    {
        m_method = std::move(value);
    }

    /** @brief Attaches the request body schema. @param value parsed schema for the JSON body.
     */
    void set_request_body(SchemaType value)
    {
        m_request_body = std::move(value);
    }

    /**
     * @brief Attaches the success response schema.
     * @param value parsed schema for the JSON response.
     */
    void set_response(SchemaType value)
    {
        m_response = std::move(value);
    }

    /** @brief Gets the URL path template. @return the path string, straight up. */
    [[nodiscard]] const std::string& get_path() const noexcept
    {
        return m_path;
    }

    /** @brief Gets the HTTP method. @return the method string, no frills. */
    [[nodiscard]] const std::string& get_method() const noexcept
    {
        return m_method;
    }

    /**
     * @brief Gets the request body schema, if this operation's even got one.
     * @return the schema, or empty if there's nothing to grab.
     */
    [[nodiscard]] const std::optional<SchemaType>& get_request_body() const noexcept
    {
        return m_request_body;
    }

    /**
     * @brief Gets the success response schema, if this operation's even got one.
     * @return the schema, or empty if there's nothing to grab.
     */
    [[nodiscard]] const std::optional<SchemaType>& get_response() const noexcept
    {
        return m_response;
    }

private:
    std::string m_path;
    std::string m_method;
    std::optional<SchemaType> m_request_body;
    std::optional<SchemaType> m_response;
};

class RouteWriter
{
public:
    /**
     * @brief Generates the whole client-side routes module in one motion: a single `Client`
     * class (inside `namespaceName`) with one member method per operation — grouped only by
     * naming (method names are prefixed with the first path segment beyond the shared prefix,
     * e.g. everything under "/api/v1/tasks/..." becomes `tasks_*` methods), not by separate
     * namespaces. `Client` owns its own `core::client::Register` as an ordinary (non-static)
     * member — no global/singleton state anywhere — so callers construct exactly one `Client`,
     * bind it once via `setRuntime()`, and every method correlates through that same instance.
     * Each generated method builds a request via
     * `core::client::Client::custom(...).build(...)`, fills in body/content-type, and
     * dispatches through the member `Register`'s `send()`.
     * @param operations every operation parsed off the OpenAPI document's `paths`.
     * @param routesModuleName name of the module to emit.
     * @param dtoModuleName name of the module the DTO types live in, used to qualify request/
     * response type names in the generated signatures.
     * @param namespaceName namespace the generated `Client` class lives in.
     * @return generated C++ source for the routes module, or an L in string form if a path has
     * no segment left over after stripping the shared prefix.
     */
    [[nodiscard]] static std::expected<std::string, std::string> write(
        const std::vector<OperationInfo>& operations,
        std::string_view routesModuleName,
        std::string_view dtoModuleName,
        std::string_view namespaceName
    )
    {
        // Stand up the module with every import a generated route method could need.
        core::generator::Generator generator{std::string{routesModuleName}};
        generator.addImport("std")
            .addImport("interfaces")
            .addImport("serde")
            .addImport("core_client")
            .addImport(std::string{dtoModuleName});

        // Figure out the segment prefix every path shares, so grouping below keys off the
        // first segment after it instead of the literal first segment.
        std::vector<std::string> all_paths;
        all_paths.reserve(operations.size());
        for (const auto& op: operations) {
            all_paths.push_back(op.get_path());
        }
        auto prefix = common_prefix_segments(all_paths);

        // Group by the first path segment after the prefix shared by every operation (e.g. all
        // paths share "/api/v1", so "/api/v1/tasks/{name}" groups under "tasks", not "api") —
        // used below only to prefix the generated method name, not to split into namespaces.
        std::unordered_map<std::string, std::vector<const OperationInfo*>> groups;
        for (const auto& op: operations) {
            auto segments = path_segments(op.get_path());
            if (segments.size() <= prefix.size()) {
                return std::unexpected{
                    std::format("path '{}' has no segment beyond the shared prefix", op.get_path())
                };
            }
            groups[segments[prefix.size()]].push_back(
                &op
            ); // FIXME(clang-tidy): unchecked operator[], consider .at()
        }

        // Sort group names so the generated method order comes out stable and deterministic —
        // straight W, no relying on unordered_map's iteration order in the output.
        std::vector<std::string> group_names;
        for (const auto& [group_name, ops]: groups) {
            (void)ops;
            group_names.push_back(group_name);
        }
        std::ranges::sort(group_names);

        auto& ns = generator.addNamespace(std::string{namespaceName});
        auto& client = ns.addClass("Client");
        client.addField("core::client::Register", "m_register");

        auto& set_runtime_fn = client.addMethod(
            "void", "setRuntime"
        ); // NOLINT(readability-identifier-naming) — matches this project's get/set/add
           // accessor naming convention (camelCase after prefix), not a real naming defect —
           // the shared clang-tidy config has no accessor exception
        set_runtime_fn.add_param(core::generator::Param{"interfaces::IClient &", "client"});
        set_runtime_fn.add_statement(
            core::generator::Stmt::raw("    m_register.set_runtime(client);\n")
        );

        auto& dispatch_fn = client.addMethod("void", "dispatch");
        dispatch_fn.add_param(core::generator::Param{"interfaces::io::IRequest &", "request"});
        dispatch_fn.add_param(core::generator::Param{"interfaces::io::IResponse &", "response"});
        dispatch_fn.add_statement(
            core::generator::Stmt::raw("    m_register.dispatch(request, response);\n")
        );

        // Emit one member method per operation, name-prefixed by its group.
        for (const auto& group_name: group_names) {
            const auto& ops = groups.at(group_name);
            for (const auto* op: ops) {
                emit_operation(client, group_name, *op, prefix.size(), dtoModuleName);
            }
        }

        return generator.render();
    }

private:
    /**
     * @brief Emits one generated member method for a single operation onto `client` — builds
     * the request via `core::client::Client::custom(...).build(...)` against `client`'s own
     * `m_register`, fills in path/method/body, and dispatches through that same `m_register`'s
     * `send()`. Split out of write()'s innermost loop body so that method's control flow stays
     * readable.
     * @param client the generated `Client` class the new method gets added to.
     * @param groupName the operation's group (first path segment beyond the shared prefix) —
     * prefixed onto the generated method name so methods from different groups never collide on
     * one flat class.
     * @param operation the operation to emit a method for.
     * @param prefixSize length of the segment prefix shared by every operation's path (see
     * common_prefix_segments()) — segments up to and including this one are skipped when
     * splitting `operation`'s path into its group-relative literal/param parts.
     * @param dtoModuleName name of the module the DTO types live in, used to qualify request/
     * response type names in the generated signature.
     */
    static void emit_operation(
        core::generator::Class& client,
        std::string_view groupName,
        const OperationInfo& operation,
        std::size_t prefixSize,
        std::string_view dtoModuleName
    )
    {
        // Split every segment past the group's own first segment into literal parts
        // (feed the function name) vs path params (feed both the signature and the
        // format string below).
        auto segments = path_segments(operation.get_path());
        std::vector<std::string> literal_tail;
        std::vector<std::string> path_params;
        for (std::size_t i = prefixSize + 1; i < segments.size(); ++i) {
            if (is_param(segments[i])) { // FIXME(clang-tidy): unchecked operator[], consider .at()
                path_params.push_back(
                    param_name(segments[i])
                ); // FIXME(clang-tidy): unchecked operator[], consider .at()
            } else {
                literal_tail.push_back(
                    segments[i]
                ); // FIXME(clang-tidy): unchecked operator[], consider .at()
            }
        }

        std::string function_name = std::format(
            "{}_{}", groupName, resolve_function_name(operation.get_method(), literal_tail)
        );
        std::string response_type = operation.get_response()
                                        ? resolve_type(*operation.get_response(), dtoModuleName)
                                        : "void";
        std::string on_response_type = operation.get_response()
                                           ? std::format("std::function<void({})>", response_type)
                                           : "std::function<void()>";

        // Signature order, straightforward motion: path params, then body (if any), then
        // the two callbacks — onError gets a no-op default so callers can skip it.
        auto& fn = client.addMethod("void", function_name);
        for (const auto& param: path_params) {
            fn.add_param(core::generator::Param{"std::string_view", param});
        }
        if (operation.get_request_body()) {
            fn.add_param(
                core::generator::Param{
                    std::format(
                        "const {} &", resolve_type(*operation.get_request_body(), dtoModuleName)
                    ),
                    "body"
                }
            );
        }
        fn.add_param(core::generator::Param{on_response_type, "onResponse"});
        fn.add_param(
            core::generator::Param{"std::function<void(std::string)>", "onError"}.set_default_value(
                "[](std::string) {}"
            )
        );

        // No path params means the path's a plain string literal; otherwise swap every
        // "{param}" placeholder for "{}" and build a std::format(...) call that fills
        // them in at request time.
        std::string path_expr;
        if (path_params.empty()) {
            path_expr = std::format("\"{}\"", operation.get_path());
        } else {
            std::string format_str = operation.get_path();
            for (const auto& param: path_params) {
                auto placeholder = std::format("{{{}}}", param);
                auto pos = format_str.find(placeholder);
                format_str.replace(pos, placeholder.size(), "{}");
            }
            path_expr = std::format("std::format(\"{}\"", format_str);
            for (const auto& param: path_params) {
                path_expr += std::format(", {}", param);
            }
            path_expr += ")";
        }

        fn.add_statement(
            core::generator::Stmt::raw(
                "    if (!m_register.has_runtime()) { throw std::runtime_error(\"Please call "
                "setRuntime() first\"); }\n"
            )
        );
        fn.add_statement(
            core::generator::Stmt::raw(
                std::format(
                    "    auto request = core::client::Client::custom(\"{}\", "
                    "{}).build(m_register.runtime());\n",
                    to_uppercase(operation.get_method()), path_expr
                )
            )
        );
        // A request body means the generated method also encodes `body` to JSON and
        // sets the content type before sending.
        if (operation.get_request_body()) {
            fn.add_statement(
                core::generator::Stmt::raw(
                    "    std::move(*request).with_content_type(\"application/json\");\n"
                )
            );
            fn.add_statement(
                core::generator::Stmt::raw(
                    "    request->set_body(serde::Ser::serialize(\"application/json\", "
                    "body));\n"
                )
            );
        }
        // Dispatch through m_register.send(), decoding the response body into Res inline when
        // there's a response to deserialize, bet, otherwise the plain status-only path.
        if (operation.get_response()) {
            fn.add_statement(
                core::generator::Stmt::raw(
                    std::format(
                        "    m_register.send(std::move(request), [on_response = "
                        "std::move(onResponse), "
                        "on_error = std::move(onError)](interfaces::io::IResponse &response) "
                        "mutable {{\n"
                        "        auto &body_view = response.get_body();\n"
                        "        std::string body;\n"
                        "        body.reserve(body_view.size());\n"
                        "        for (auto byte : body_view) {{ "
                        "body.push_back(static_cast<char>(byte)); }}\n"
                        "        auto result = "
                        "serde::Ser::deserialize<{}>(response.get_content_type(), body);\n"
                        "        if (result.has_value()) {{ on_response(std::move(*result)); "
                        "}}\n"
                        "        else {{ on_error(result.error()); }}\n"
                        "    }});\n",
                        response_type
                    )
                )
            );
        } else {
            fn.add_statement(
                core::generator::Stmt::raw(
                    "    m_register.send(std::move(request), [on_response = "
                    "std::move(onResponse), "
                    "on_error = std::move(onError)](interfaces::io::IResponse &response) "
                    "mutable {\n"
                    "        if (response.is_success()) { on_response(); }\n"
                    "        else { on_error(std::string{response.get_status_text()}); }\n"
                    "    });\n"
                )
            );
        }
    }

    // Mirrors DtoWriter::resolve_cpp_type() — must handle every SchemaKind, not just Ref,
    // because a top-level operation response can itself be Array (e.g. GET /metadata/tasks
    // responds with std::vector<TaskDef>, whose SchemaType kind is Array-of-Ref, not a bare
    // Ref).
    /**
     * @brief Maps a parsed SchemaType to the C++ type string generated function signatures
     * actually use — same motion as DtoWriter::resolve_cpp_type() but qualifies Ref types with
     * the DTO module name, since routes live in a different module than the DTOs they
     * reference.
     * @param type schema node to resolve (may be a top-level Array, e.g. a list-endpoint
     * response, not just a bare Ref).
     * @param dtoModule name of the module the referenced DTO types live in.
     * @return C++ type name as a string, qualified and ready to drop into generated code.
     */
    [[nodiscard]] static std::string
    resolve_type(const SchemaType& type, std::string_view dtoModule)
    {
        // Resolve the base type first, same shape as DtoWriter's version — Ref gets qualified
        // with the DTO module name since routes live in a separate module.
        std::string base;
        switch (type.get_kind()) {
            case SchemaKind::REF:
                base = std::format("{}::{}", dtoModule, type.get_ref());
                break;
            case SchemaKind::ARRAY:
                base = std::format("std::vector<{}>", resolve_type(type.get_items(), dtoModule));
                break;
            case SchemaKind::STRING:
                base = "std::string";
                break;
            case SchemaKind::INTEGER:
                base = "std::int64_t";
                break;
            case SchemaKind::NUMBER:
                base = "double";
                break;
            case SchemaKind::BOOLEAN:
                base = "bool";
                break;
            case SchemaKind::OBJECT:
                base = "std::string"; // anonymous inline object — not expected from our own server
                break;
        }
        // Then wrap in optional if the schema says null's on the table.
        return type.get_nullable() ? std::format("std::optional<{}>", base) : base;
    }

    /**
     * @brief Splits a URL path into its non-empty slash-delimited segments — leading, trailing,
     * doubled slashes just get skipped, zero empty-string segments sneak into the output.
     * @param path URL path template (e.g. "/api/v1/tasks/{name}").
     * @return each segment in order (e.g. {"api", "v1", "tasks", "{name}"}).
     */
    [[nodiscard]] static std::vector<std::string> path_segments(std::string_view path)
    {
        std::vector<std::string> segments;
        std::size_t start = 0;
        // Walk slash to slash, only keeping non-empty parts — that's what drops leading,
        // trailing, and doubled slashes from the output.
        while (start < path.size()) {
            auto slash = path.find('/', start);
            auto part = path.substr(
                start, slash == std::string_view::npos ? std::string_view::npos : slash - start
            );
            if (!part.empty()) {
                segments.emplace_back(part);
            }
            if (slash == std::string_view::npos) {
                break;
            }
            start = slash + 1;
        }
        return segments;
    }

    /**
     * @brief Checks whether a path segment is an OpenAPI path parameter placeholder, e.g.
     * "{name}" — quick vibe check on the braces.
     * @param segment single path segment to check.
     * @return true if the segment is wrapped in curly braces.
     */
    [[nodiscard]] static bool is_param(const std::string& segment)
    {
        return segment.size() > 1 && segment.front() == '{' && segment.back() == '}';
    }

    /**
     * @brief Strips the curly braces off a path parameter placeholder, leaving just the name.
     * @param segment a segment for which is_param() is true (e.g. "{name}").
     * @return the bare parameter name (e.g. "name").
     */
    [[nodiscard]] static std::string param_name(const std::string& segment)
    {
        return segment.substr(1, segment.size() - 2);
    }

    /**
     * @brief Builds the generated function's name from the HTTP method plus every literal
     * (non-param) path segment beyond the group prefix, underscore-joined — straightforward
     * motion.
     * @warning Method names that collide with a C++ keyword (like "delete") get a trailing
     * underscore tacked on — miss that and the generated code just won't compile, straight L.
     * @param method HTTP method for the operation (e.g. "delete").
     * @param literal_tail literal path segments after the group's first segment, in path order.
     * @return generated function name (e.g. "delete_status" or "delete_status_" if it collides
     * with a keyword).
     */
    [[nodiscard]] static std::string
    resolve_function_name(std::string_view method, const std::vector<std::string>& literal_tail)
    {
        // Build the raw name first — method plus every literal segment, underscore-joined.
        std::string name{method};
        for (const auto& part: literal_tail) {
            name += "_" + part;
        }
        // "delete" (and any other C++ keyword that happens to be an HTTP method name) can't be
        // used as a function name — append a trailing underscore, same convention as e.g.
        // "class_".
        static const std::unordered_set<std::string> KEYWORDS{"delete", "new", "class", "template"};
        if (KEYWORDS.contains(name)) {
            name += "_";
        }
        return name;
    }

    /**
     * @brief Uppercases every character in the given string, no exceptions.
     * @param value string to convert.
     * @return uppercased copy of `value`.
     */
    [[nodiscard]] static std::string to_uppercase(std::string_view value)
    {
        std::string result{value};
        std::ranges::transform(result, result.begin(), [](unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });
        return result;
    }

    // OpenAPI's own operations (from utils::openapi's server-side generator) group by whatever
    // segment comes right after the API's common prefix (e.g. "/api/v1/tasks" -> "tasks"), not
    // by the literal first segment — every path in a typical API shares the same "/api/v1"
    // prefix, so grouping by the bare first segment would collapse every operation into one
    // namespace. Strip the longest segment prefix shared by every operation's path first.
    /**
     * @brief Finds the longest segment prefix shared by every operation's path, so grouping can
     * key off the first segment *after* that shared prefix instead of the literal first segment
     * — otherwise everything collapses into one namespace, e.g. every path sharing "/api/v1"
     * would be straight chaos. Always reserves at least one trailing segment per path, so a
     * "collection root" path never gets stripped down to nothing.
     * @param paths every operation's path template.
     * @return the shared prefix as a list of segments (possibly empty).
     */
    [[nodiscard]] static std::vector<std::string>
    common_prefix_segments(const std::vector<std::string>& paths)
    {
        if (paths.empty()) {
            return {};
        }
        // Segment every path up front and track the shortest one — that bounds how long the
        // shared prefix could possibly be.
        std::vector<std::vector<std::string>> all_segments;
        std::size_t min_len = std::numeric_limits<std::size_t>::max();
        for (const auto& path: paths) {
            auto segments = path_segments(path);
            min_len = std::min(min_len, segments.size());
            all_segments.push_back(std::move(segments));
        }
        // A "collection root" path (e.g. "/api/v1/tasks", shared as a literal prefix by
        // "/api/v1/tasks/{name}") would otherwise let the naive longest-common-prefix consume
        // that path's own only segment, leaving it with no group key at all — reserve at least
        // one trailing segment per path.
        std::size_t max_prefix = min_len > 0 ? min_len - 1 : 0;

        // Seed the candidate prefix off the first path, capped at max_prefix — clean starting
        // motion before the shrink pass below.
        auto prefix = all_segments.front();
        if (prefix.size() > max_prefix) {
            prefix.resize(max_prefix);
        }
        // Then shrink it against every other path — first mismatch (or a param segment, which
        // never counts as shared) cuts the candidate down right there.
        for (const auto& segments: all_segments) {
            std::size_t common = 0;
            while (common < prefix.size() && common < segments.size() &&
                   prefix[common] == segments[common] &&
                   !is_param(
                       prefix[common]
                   )) { // FIXME(clang-tidy): unchecked operator[], consider .at()
                ++common;
            }
            prefix.resize(common);
        }
        return prefix;
    }
};

} // namespace congelado::client

#ifdef CONGELADO_TEST
namespace openapi_gen_route_writer_tests {
using namespace boost::ut;
using congelado::client::OperationInfo;
using congelado::client::RouteWriter;
using congelado::client::SchemaKind;
using congelado::client::SchemaType;

[[nodiscard]] OperationInfo make_operation(std::string method, std::string path)
{
    OperationInfo op;
    op.set_method(std::move(method));
    op.set_path(std::move(path));
    return op;
}

[[nodiscard]] SchemaType make_primitive(SchemaKind kind, bool nullable = false)
{
    SchemaType schema;
    schema.set_kind(kind);
    schema.set_nullable(nullable);
    return schema;
}

[[nodiscard]] SchemaType make_ref(std::string name)
{
    SchemaType schema;
    schema.set_kind(SchemaKind::REF);
    schema.set_ref(std::move(name));
    return schema;
}

[[nodiscard]] SchemaType make_array(SchemaType items)
{
    SchemaType schema;
    schema.set_kind(SchemaKind::ARRAY);
    schema.set_items(std::move(items));
    return schema;
}

suite<"OperationInfo"> operation_info_suite = [] {
    "default state has empty path/method and no body/response"_test = [] {
        OperationInfo op;

        expect(op.get_path().empty());
        expect(op.get_method().empty());
        expect(not op.get_request_body().has_value());
        expect(not op.get_response().has_value());
    };

    "set_path/get_path round-trip"_test = [] {
        OperationInfo op;
        op.set_path("/api/v1/tasks/{id}");

        expect(op.get_path() == "/api/v1/tasks/{id}");
    };

    "set_method/get_method round-trip"_test = [] {
        OperationInfo op;
        op.set_method("post");

        expect(op.get_method() == "post");
    };

    "set_request_body/get_request_body round-trip"_test = [] {
        OperationInfo op;
        op.set_request_body(make_ref("TaskCreate"));

        expect(op.get_request_body().has_value()) << fatal;
        expect(op.get_request_body()->get_kind() == SchemaKind::REF);
        expect(op.get_request_body()->get_ref() == "TaskCreate");
    };

    "set_response/get_response round-trip"_test = [] {
        OperationInfo op;
        op.set_response(make_primitive(SchemaKind::INTEGER));

        expect(op.get_response().has_value()) << fatal;
        expect(op.get_response()->get_kind() == SchemaKind::INTEGER);
    };
};

suite<"RouteWriter"> route_writer_suite = [] {
    "write: a single no-param GET operation renders full request wiring, no response type"_test =
        [] {
            std::vector<OperationInfo> ops{make_operation("get", "/api/v1/tasks")};

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(result.has_value()) << fatal;
            expect(result->contains("export namespace client {"));
            expect(result->contains("class Client {"));
            expect(result->contains("core::client::Register m_register;"));
            expect(result->contains(
                "void tasks_get(std::function<void()> onResponse, "
                "std::function<void(std::string)> "
                "onError = [](std::string) {})"
            ));
            expect(result->contains(
                "core::client::Client::custom(\"GET\", "
                "\"/api/v1/tasks\").build(m_register.runtime())"
            ));
            expect(result->contains("m_register.send(std::move(request), "));
            expect(not result->contains("with_content_type"));
        };

    "write: a path param plus trailing literal segment builds a std::string_view param and a "
    "std::format path expression, method name joins group + method + tail with underscores"_test =
        [] {
            std::vector<OperationInfo> ops{
                make_operation("get", "/api/v1/tasks"),
                make_operation("post", "/api/v1/tasks/{id}/enqueue")
            };

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(result.has_value()) << fatal;
            expect(result->contains(
                "void tasks_post_enqueue(std::string_view id, std::function<void()> "
                "onResponse, "
                "std::function<void(std::string)> onError = [](std::string) {})"
            ));
            expect(result->contains("std::format(\"/api/v1/tasks/{}/enqueue\", id)"));
            expect(result->contains("core::client::Client::custom(\"POST\", "));
        };

    "write: a multi-segment literal tail joins every part into the method name"_test = [] {
        std::vector<OperationInfo> ops{
            make_operation("get", "/api/v1/tasks"),
            make_operation("get", "/api/v1/tasks/{id}/comments/all")
        };

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains("void tasks_get_comments_all(std::string_view id, "));
    };

    "write: a request body emits a body param, content-type wiring, and a serialize call"_test =
        [] {
            OperationInfo op = make_operation("post", "/api/v1/tasks");
            op.set_request_body(make_ref("TaskCreate"));
            std::vector<OperationInfo> ops{op};

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(result.has_value()) << fatal;
            expect(result->contains(
                "void tasks_post(const dto_mod::TaskCreate &body, std::function<void()> "
                "onResponse, "
                "std::function<void(std::string)> onError = [](std::string) {})"
            ));
            expect(result->contains(".with_content_type(\"application/json\");"));
            expect(result->contains(
                "request->set_body(serde::Ser::serialize(\"application/json\", body));"
            ));
        };

    "write: a $ref response is qualified with the dto module and decoded via deserialize<T>"_test =
        [] {
            OperationInfo op = make_operation("get", "/api/v1/tasks/{id}");
            op.set_response(make_ref("TaskDef"));
            std::vector<OperationInfo> ops{make_operation("get", "/api/v1/tasks"), op};

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(result.has_value()) << fatal;
            expect(result->contains(
                "void tasks_get(std::string_view id, std::function<void(dto_mod::TaskDef)> "
                "onResponse, "
                "std::function<void(std::string)> onError = [](std::string) {})"
            ));
            expect(result->contains(
                "serde::Ser::deserialize<dto_mod::TaskDef>(response.get_content_type(), body);"
            ));
        };

    "write: an array-of-$ref response resolves to std::vector<dto_mod::T>"_test = [] {
        OperationInfo op = make_operation("get", "/api/v1/tasks");
        op.set_response(make_array(make_ref("TaskDef")));
        std::vector<OperationInfo> ops{op};

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains("std::function<void(std::vector<dto_mod::TaskDef>)> onResponse"));
        expect(result->contains("serde::Ser::deserialize<std::vector<dto_mod::TaskDef>>("));
    };

    "write: a nullable response wraps the resolved type in std::optional"_test = [] {
        OperationInfo op = make_operation("get", "/api/v1/tasks/{id}");
        SchemaType nullable_ref;
        nullable_ref.set_kind(SchemaKind::REF);
        nullable_ref.set_ref("TaskDef");
        nullable_ref.set_nullable(true);
        op.set_response(nullable_ref);
        std::vector<OperationInfo> ops{make_operation("get", "/api/v1/tasks"), op};

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains("std::optional<dto_mod::TaskDef>"));
    };

    "write: Integer/Number/Boolean/Object responses resolve like DtoWriter's own type mapping"_test =
        [] {
            OperationInfo int_op = make_operation("get", "/count");
            int_op.set_response(make_primitive(SchemaKind::INTEGER));
            auto int_result = RouteWriter::write(
                std::vector<OperationInfo>{int_op}, "routes_mod", "dto_mod", "client"
            );
            expect(int_result.has_value()) << fatal;
            expect(int_result->contains("serde::Ser::deserialize<std::int64_t>("));

            OperationInfo num_op = make_operation("get", "/ratio");
            num_op.set_response(make_primitive(SchemaKind::NUMBER));
            auto num_result = RouteWriter::write(
                std::vector<OperationInfo>{num_op}, "routes_mod", "dto_mod", "client"
            );
            expect(num_result.has_value()) << fatal;
            expect(num_result->contains("serde::Ser::deserialize<double>("));

            OperationInfo bool_op = make_operation("get", "/active");
            bool_op.set_response(make_primitive(SchemaKind::BOOLEAN));
            auto bool_result = RouteWriter::write(
                std::vector<OperationInfo>{bool_op}, "routes_mod", "dto_mod", "client"
            );
            expect(bool_result.has_value()) << fatal;
            expect(bool_result->contains("serde::Ser::deserialize<bool>("));

            OperationInfo object_op = make_operation("get", "/blob");
            object_op.set_response(make_primitive(SchemaKind::OBJECT));
            auto object_result = RouteWriter::write(
                std::vector<OperationInfo>{object_op}, "routes_mod", "dto_mod", "client"
            );
            expect(object_result.has_value()) << fatal;
            expect(object_result->contains("serde::Ser::deserialize<std::string>("));
        };

    "write: groups sort alphabetically by method-name prefix, independent of insertion order"_test =
        [] {
            std::vector<OperationInfo> ops{
                make_operation("get", "/api/v1/users"), make_operation("get", "/api/v1/tasks")
            };

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(result.has_value()) << fatal;
            auto tasks_pos = result->find("tasks_get(");
            auto users_pos = result->find("users_get(");
            expect(tasks_pos != std::string::npos) << fatal;
            expect(users_pos != std::string::npos) << fatal;
            expect(tasks_pos < users_pos);
        };

    "write: a method colliding with a C++ keyword gets a trailing underscore"_test = [] {
        std::vector<OperationInfo> ops{
            make_operation("get", "/api/v1/tasks"), make_operation("delete", "/api/v1/tasks/{id}")
        };

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains(
            "void tasks_delete_(std::string_view id, std::function<void()> onResponse, "
            "std::function<void(std::string)> onError = [](std::string) {})"
        ));
        expect(result->contains("core::client::Client::custom(\"DELETE\", "));
    };

    "write: errors when a path has no segment beyond the shared prefix (e.g. a bare root path)"_test =
        [] {
            std::vector<OperationInfo> ops{make_operation("get", "/")};

            auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

            expect(not result.has_value()) << fatal;
            expect(result.error() == "path '/' has no segment beyond the shared prefix");
        };

    "write: leading, trailing, and doubled slashes are skipped when segmenting a path, but the "
    "original path string is still what gets emitted as the literal"_test = [] {
        std::vector<OperationInfo> ops{make_operation("get", "/api/v1//tasks/")};

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains("void tasks_get("));
        expect(result->contains(".build(m_register.runtime())"));
        expect(result->contains("\"/api/v1//tasks/\""));
    };

    // Locks in RouteWriter::write()'s current, unmodified behavior: when the first segment
    // after the shared literal prefix is itself a path parameter (not a literal), that
    // segment gets consumed as the group name for the generated method-name prefix (raw
    // "{param}" placeholder text, braces included -- "{id}_get_b", not a valid C++ identifier)
    // AND emit_operation()'s tail loop starts one segment past it, so the param is never
    // picked up as a method parameter either -- the generated method ends up with no "id"
    // parameter at all, and the literal "{id}" placeholder text leaks verbatim into the
    // hardcoded path string with no std::format substitution. Looks like a real bug
    // (compounded by the fact the id is silently dropped, not just misnamed). Not fixed
    // here -- just documenting the as-written behavior this pass must not change.
    "write: a path param immediately after the shared prefix becomes the (invalid) method-name "
    "prefix and is silently dropped as a parameter -- looks like a bug, not fixed here"_test = [] {
        std::vector<OperationInfo> ops{
            make_operation("get", "/a/{id}/b"), make_operation("post", "/a/{id}/c")
        };

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->contains("void {id}_get_b(std::function<void()> onResponse, "));
        expect(not result->contains("std::string_view id"));
        expect(result->contains("\"/a/{id}/b\""));
    };

    "write: an empty operations list still renders a valid module with just the empty Client "
    "shell (setRuntime/dispatch, no route methods)"_test = [] {
        std::vector<OperationInfo> ops;

        auto result = RouteWriter::write(ops, "routes_mod", "dto_mod", "client");

        expect(result.has_value()) << fatal;
        expect(result->starts_with("export module routes_mod;\n"));
        expect(result->contains("class Client {"));
        expect(result->contains("void setRuntime(interfaces::IClient &client)"));
        expect(result->contains(
            "void dispatch(interfaces::io::IRequest &request, "
            "interfaces::io::IResponse &response)"
        ));
    };
};

} // namespace openapi_gen_route_writer_tests
#endif

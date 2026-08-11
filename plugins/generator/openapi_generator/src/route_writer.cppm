export module openapi_generator_plugin:route_writer;

import std;
import core_generator;
import :schema_model;

export namespace congelado::client {

class OperationInfo {
  public:
    /** @brief Builds an empty operation — no path/method/body/response set yet, blank canvas. */
    OperationInfo() = default;

    /**
     * @brief Locks in the URL path template (e.g. "/api/v1/tasks/{name}").
     * @param value path string.
     */
    void set_path(std::string value) { m_path = std::move(value); }
    /** @brief Locks in the HTTP method (e.g. "get", "post"). @param value method string. */
    void set_method(std::string value) { m_method = std::move(value); }
    /** @brief Attaches the request body schema. @param value parsed schema for the JSON body. */
    void set_request_body(SchemaType value) { m_request_body = std::move(value); }
    /**
     * @brief Attaches the success response schema.
     * @param value parsed schema for the JSON response.
     */
    void set_response(SchemaType value) { m_response = std::move(value); }

    /** @brief Gets the URL path template. @return the path string, straight up. */
    [[nodiscard]] const std::string &get_path() const noexcept { return m_path; }
    /** @brief Gets the HTTP method. @return the method string, no frills. */
    [[nodiscard]] const std::string &get_method() const noexcept { return m_method; }
    /**
     * @brief Gets the request body schema, if this operation's even got one.
     * @return the schema, or empty if there's nothing to grab.
     */
    [[nodiscard]] const std::optional<SchemaType> &get_request_body() const noexcept {
        return m_request_body;
    }
    /**
     * @brief Gets the success response schema, if this operation's even got one.
     * @return the schema, or empty if there's nothing to grab.
     */
    [[nodiscard]] const std::optional<SchemaType> &get_response() const noexcept {
        return m_response;
    }

  private:
    std::string m_path;
    std::string m_method;
    std::optional<SchemaType> m_request_body;
    std::optional<SchemaType> m_response;
};

class RouteWriter {
  public:
    /**
     * @brief Generates the whole client-side routes module in one motion: one function per
     * operation, grouped into a namespace per the first path segment beyond the shared prefix
     * (e.g. everything under "/api/v1/tasks/..." lands in `<namespaceName>::tasks`). Each
     * generated function builds a request via ClientRuntime::new_request(), fills in
     * path/method/body, and dispatches through ClientRuntime::send — that's the whole client
     * codegen story, no cap.
     * @param operations every operation parsed off the OpenAPI document's `paths`.
     * @param routesModuleName name of the module to emit.
     * @param dtoModuleName name of the module the DTO types live in, used to qualify request/
     * response type names in the generated signatures.
     * @param namespaceName base namespace the per-group namespaces nest under.
     * @return generated C++ source for the routes module, or an L in string form if a path has
     * no segment left over after stripping the shared prefix.
     */
    [[nodiscard]] static std::expected<std::string, std::string>
    write(const std::vector<OperationInfo> &operations, std::string_view routesModuleName,
         std::string_view dtoModuleName, std::string_view namespaceName) {
        // Stand up the module with every import a generated route function could need.
        core::generator::Generator generator{std::string{routesModuleName}};
        generator.addImport("std")
            .addImport("interfaces")
            .addImport("serde")
            .addImport("congelado_client")
            .addImport(std::string{dtoModuleName});

        // Figure out the segment prefix every path shares, so grouping below keys off the
        // first segment after it instead of the literal first segment.
        std::vector<std::string> all_paths;
        all_paths.reserve(operations.size());
        for (const auto &op : operations) {
            all_paths.push_back(op.get_path());
        }
        auto prefix = common_prefix_segments(all_paths);

        // Group by the first path segment after the prefix shared by every operation (e.g. all
        // paths share "/api/v1", so "/api/v1/tasks/{name}" groups under "tasks", not "api").
        std::unordered_map<std::string, std::vector<const OperationInfo *>> groups;
        for (const auto &op : operations) {
            auto segments = path_segments(op.get_path());
            if (segments.size() <= prefix.size()) {
                return std::unexpected{
                    std::format("path '{}' has no segment beyond the shared prefix", op.get_path())};
            }
            groups[segments[prefix.size()]].push_back(&op);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }

        // Sort group names so the generated namespaces come out in a stable, deterministic
        // order — straight W, no relying on unordered_map's iteration order in the output.
        std::vector<std::string> group_names;
        for (const auto &[group_name, ops] : groups) {
            (void)ops;
            group_names.push_back(group_name);
        }
        std::ranges::sort(group_names);

        // Emit one namespace per group, one function per operation inside it.
        for (const auto &group_name : group_names) {
            const auto &ops = groups.at(group_name);
            auto &ns = generator.addNamespace(std::format("{}::{}", namespaceName, group_name));
            for (const auto *op : ops) {
                emit_operation(ns, *op, prefix.size(), dtoModuleName);
            }
        }

        return generator.render();
    }

  private:
    /**
     * @brief Emits one generated function for a single operation into `routeNamespace` — builds
     * the request via ClientRuntime::new_request(), fills in path/method/body, and dispatches
     * through ClientRuntime::send. Split out of write()'s innermost loop body so that method's
     * control flow stays readable; behavior is unchanged from the inline version.
     * @param routeNamespace the namespace the generated function gets added to.
     * @param operation the operation to emit a function for.
     * @param prefixSize length of the segment prefix shared by every operation's path (see
     * common_prefix_segments()) — segments up to and including this one are skipped when
     * splitting `operation`'s path into its group-relative literal/param parts.
     * @param dtoModuleName name of the module the DTO types live in, used to qualify request/
     * response type names in the generated signature.
     */
    static void emit_operation(core::generator::Namespace &routeNamespace, const OperationInfo &operation,
                               std::size_t prefixSize, std::string_view dtoModuleName) {
        // Split every segment past the group's own first segment into literal parts
        // (feed the function name) vs path params (feed both the signature and the
        // format string below).
        auto segments = path_segments(operation.get_path());
        std::vector<std::string> literal_tail;
        std::vector<std::string> path_params;
        for (std::size_t i = prefixSize + 1; i < segments.size(); ++i) {
            if (is_param(segments[i])) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                path_params.push_back(param_name(segments[i]));  // FIXME(clang-tidy): unchecked operator[], consider .at()
            } else {
                literal_tail.push_back(segments[i]);  // FIXME(clang-tidy): unchecked operator[], consider .at()
            }
        }

        std::string function_name = resolve_function_name(operation.get_method(), literal_tail);
        std::string response_type =
            operation.get_response() ? resolve_type(*operation.get_response(), dtoModuleName) : "void";
        std::string on_response_type =
            operation.get_response() ? std::format("std::function<void({})>", response_type)
                                     : "std::function<void()>";

        // Signature order, straightforward motion: path params, then body (if any), then
        // the two callbacks — onError gets a no-op default so callers can skip it.
        auto &fn = routeNamespace.addFunction("void", function_name);
        for (const auto &param : path_params) {
            fn.add_param(core::generator::Param{"std::string_view", param});
        }
        if (operation.get_request_body()) {
            fn.add_param(core::generator::Param{
                std::format("const {} &", resolve_type(*operation.get_request_body(), dtoModuleName)),
                "body"});
        }
        fn.add_param(core::generator::Param{on_response_type, "onResponse"});
        fn.add_param(core::generator::Param{"std::function<void(std::string)>", "onError"}
                        .set_default_value("[](std::string) {}"));

        // No path params means the path's a plain string literal; otherwise swap every
        // "{param}" placeholder for "{}" and build a std::format(...) call that fills
        // them in at request time.
        std::string path_expr;
        if (path_params.empty()) {
            path_expr = std::format("\"{}\"", operation.get_path());
        } else {
            std::string format_str = operation.get_path();
            for (const auto &param : path_params) {
                auto placeholder = std::format("{{{}}}", param);
                auto pos = format_str.find(placeholder);
                format_str.replace(pos, placeholder.size(), "{}");
            }
            path_expr = std::format("std::format(\"{}\"", format_str);
            for (const auto &param : path_params) {
                path_expr += std::format(", {}", param);
            }
            path_expr += ")";
        }

        fn.add_statement(core::generator::Stmt::raw(
            "    auto request = congelado::client::ClientRuntime::new_request();\n"));
        fn.add_statement(core::generator::Stmt::raw(std::format(
            "    std::move(*request).with_method(\"{}\").with_path({});\n",
            to_uppercase(operation.get_method()), path_expr)));
        // A request body means the generated function also encodes `body` to JSON and
        // sets the content type before sending.
        if (operation.get_request_body()) {
            fn.add_statement(core::generator::Stmt::raw(
                "    std::move(*request).with_content_type(\"application/json\");\n"));
            fn.add_statement(core::generator::Stmt::raw(
                "    request->set_body(serde::Ser::serialize(\"application/json\", body));\n"));
        }
        // Dispatch through the typed send<Res>() overload when there's a response to
        // deserialize, bet, otherwise the plain status-only overload.
        if (operation.get_response()) {
            fn.add_statement(core::generator::Stmt::raw(std::format(
                "    congelado::client::ClientRuntime::send<{}>(std::move(request), "
                "std::move(onResponse), std::move(onError));\n",
                response_type)));
        } else {
            fn.add_statement(core::generator::Stmt::raw(
                "    congelado::client::ClientRuntime::send(std::move(request), "
                "std::move(onResponse), std::move(onError));\n"));
        }
    }

    // Mirrors DtoWriter::resolve_cpp_type() — must handle every SchemaKind, not just Ref,
    // because a top-level operation response can itself be Array (e.g. GET /metadata/tasks
    // responds with std::vector<TaskDef>, whose SchemaType kind is Array-of-Ref, not a bare Ref).
    /**
     * @brief Maps a parsed SchemaType to the C++ type string generated function signatures
     * actually use — same motion as DtoWriter::resolve_cpp_type() but qualifies Ref types with
     * the DTO module name, since routes live in a different module than the DTOs they reference.
     * @param type schema node to resolve (may be a top-level Array, e.g. a list-endpoint
     * response, not just a bare Ref).
     * @param dtoModule name of the module the referenced DTO types live in.
     * @return C++ type name as a string, qualified and ready to drop into generated code.
     */
    [[nodiscard]] static std::string resolve_type(const SchemaType &type, std::string_view dtoModule) {
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
    [[nodiscard]] static std::vector<std::string> path_segments(std::string_view path) {
        std::vector<std::string> segments;
        std::size_t start = 0;
        // Walk slash to slash, only keeping non-empty parts — that's what drops leading,
        // trailing, and doubled slashes from the output.
        while (start < path.size()) {
            auto slash = path.find('/', start);
            auto part = path.substr(start, slash == std::string_view::npos ? std::string_view::npos
                                                                           : slash - start);
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
    [[nodiscard]] static bool is_param(const std::string &segment) {
        return segment.size() > 1 && segment.front() == '{' && segment.back() == '}';
    }

    /**
     * @brief Strips the curly braces off a path parameter placeholder, leaving just the name.
     * @param segment a segment for which is_param() is true (e.g. "{name}").
     * @return the bare parameter name (e.g. "name").
     */
    [[nodiscard]] static std::string param_name(const std::string &segment) {
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
    resolve_function_name(std::string_view method, const std::vector<std::string> &literal_tail) {
        // Build the raw name first — method plus every literal segment, underscore-joined.
        std::string name{method};
        for (const auto &part : literal_tail) {
            name += "_" + part;
        }
        // "delete" (and any other C++ keyword that happens to be an HTTP method name) can't be
        // used as a function name — append a trailing underscore, same convention as e.g. "class_".
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
    [[nodiscard]] static std::string to_uppercase(std::string_view value) {
        std::string result{value};
        std::ranges::transform(result, result.begin(),
                               [](unsigned char character) { return static_cast<char>(std::toupper(character)); });
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
    common_prefix_segments(const std::vector<std::string> &paths) {
        if (paths.empty()) {
            return {};
        }
        // Segment every path up front and track the shortest one — that bounds how long the
        // shared prefix could possibly be.
        std::vector<std::vector<std::string>> all_segments;
        std::size_t min_len = std::numeric_limits<std::size_t>::max();
        for (const auto &path : paths) {
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
        for (const auto &segments : all_segments) {
            std::size_t common = 0;
            while (common < prefix.size() && common < segments.size() &&
                  prefix[common] == segments[common] && !is_param(prefix[common])) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
                ++common;
            }
            prefix.resize(common);
        }
        return prefix;
    }
};

} // namespace congelado::client

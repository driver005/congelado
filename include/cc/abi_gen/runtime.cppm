module;

#include <cstdio>

export module cc_abi_gen:runtime;

import std;
import cc_abi_gen_classify;
import cc_abi_gen_parser;
import :writer;

export namespace cc_abi_gen {

// Drives one generator run: parse headers, place every module, write classify output.
class Runtime
{
public:
    using PathCallback = std::function<void(std::filesystem::path&)>;

    Runtime(
        std::filesystem::path&& repo_root,
        std::string&& base_folder,
        std::string&& namespace_name
    ) :
        m_repo_root{std::move(repo_root)},
        m_base_folder{std::move(base_folder)},
        m_namespace_name{std::move(namespace_name)}
    {
        m_parser.set_namespace_name(std::string{m_namespace_name});
    }

    ~Runtime() = default;
    // m_parser keeps a reference to m_registry, so the object stays where it was built.
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    void overwrite_path_callback(PathCallback&& callback)
    {
        m_path_callback = std::move(callback);
    }

    std::expected<void, std::string> parse_directory(std::string_view headers_path)
    {
        if (m_repo_root.empty()) {
            return std::unexpected{"repo_root not set"};
        }

        return m_parser.parse_directory(m_repo_root, headers_path);
    }

    // Both layers of every vtable, every value wrapper, the runtime bases, every base module.
    std::expected<void, std::string> generate()
    {
        if (m_registry.empty()) {
            return std::unexpected{"no models parsed"};
        }

        for (const auto& [C_NAME, MODEL]: m_registry) {
            if (MODEL.get_kind() == classify::Kind::VTABLE) {
                auto builder = generate_module(MODEL, parser::Mode::BUILDER);
                if (!builder) {
                    return builder;
                }
            }

            auto sonic = generate_module(MODEL, parser::Mode::SONIC);
            if (!sonic) {
                return sonic;
            }
        }

        auto runtime = generate_runtime();
        if (!runtime) {
            return runtime;
        }

        return generate_base_modules();
    }

    void set_repo_root(std::filesystem::path&& repo_root) noexcept
    {
        m_repo_root = std::move(repo_root);
    }

    void set_output_dir(std::filesystem::path&& output_dir) noexcept
    {
        m_output_dir = std::move(output_dir);
    }

    void set_base_folder(std::string&& base_folder) noexcept
    {
        m_base_folder = std::move(base_folder);
    }

    void set_namespace_name(std::string&& namespace_name)
    {
        m_namespace_name = std::move(namespace_name);
        m_parser.set_namespace_name(std::string{m_namespace_name});
    }

private:
    // One builder or sonic module file of one vtable, or the sonic wrapper of one value.
    std::expected<void, std::string>
    generate_module(const classify::Classify& model, parser::Mode mode)
    {
        const auto DIRECTORY = module_directory(model, mode);
        const std::string PARTITION = partition_name(model);

        auto module_name = join(DIRECTORY, "_", false);
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto imports = collect_imports(model, mode, DIRECTORY);
        if (!imports) {
            return imports;
        }

        auto cls = m_parser.get_visitor().build(model, mode);
        if (!cls) {
            return std::unexpected(std::move(cls.error()));
        }

        std::string fragment = banner("//", model.get_header_path()) + "\nmodule;\n\n";
        for (const std::string& header: dependency_includes(model, mode)) {
            fragment += std::format("#include \"{}\"\n", header);
        }
        fragment += '\n';

        classify::ModuleBuilder module{std::format("{}:{}", *module_name, PARTITION)};
        (void)module.set_global_fragment(std::move(fragment)).set_import("std");
        for (const std::string& line: m_scratch_imports) {
            (void)module.set_raw_import(line);
        }
        (void)module.set_namespace(m_namespace_name)
            .set_namespace(std::string{layer(mode)})
            .set_class(std::move(*cls));

        auto written = write_partition(DIRECTORY, PARTITION, module);
        if (!written) {
            return written;
        }

        m_dependencies_by_folder[DIRECTORY].insert(
            m_scratch_dependencies.begin(),
            m_scratch_dependencies.end()
        );

        return {};
    }

    // The template bases every generated class derives from, in include/cc/<ns>/runtime/<layer>.
    std::expected<void, std::string> generate_runtime()
    {
        for (auto&& [MODE, PARTITION, CLS]: {
                 std::tuple{parser::Mode::SONIC, "runtime", sonic_runtime_class()},
                 std::tuple{parser::Mode::SONIC, "enum_value", enum_value_class()},
                 std::tuple{parser::Mode::SONIC, "struct_value", struct_value_class()},
                 std::tuple{parser::Mode::BUILDER, "runtime", builder_runtime_class()},
             }) {
            const auto DIRECTORY = runtime_directory(MODE);
            auto module_name = join(DIRECTORY, "_", false);
            if (!module_name) {
                return std::unexpected(std::move(module_name.error()));
            }

            classify::ModuleBuilder module{std::format("{}:{}", *module_name, PARTITION)};
            (void)module.set_global_fragment(banner("//", "") + '\n')
                .set_import("std")
                .set_namespace(m_namespace_name)
                .set_namespace(std::string{layer(MODE)})
                .set_class(CLS);

            auto written = write_partition(DIRECTORY, PARTITION, module);
            if (!written) {
                return written;
            }
        }

        return {};
    }

    // `ice::sonic::Runtime<Ops, Handle>`: the ops table and the C handle of one caller object.
    static classify::Classify sonic_runtime_class()
    {
        classify::Classify cls{"Runtime"};
        cls.set_template("typename OpsStruct, typename HandleStruct")
            .set_public_raw("using Ops = OpsStruct;\n");

        cls.set_function(constructor("Runtime")
                             .set_explicit()
                             .set_param("const OpsStruct*", "ops")
                             .set_initializer("m_ops{ops}"));
        cls.set_function(
            constructor("Runtime")
                .set_param("const OpsStruct*", "ops")
                .set_param("const HandleStruct*", "handle")
                .set_initializer("m_ops{ops}")
                .set_body_from_raw_string("if (handle != nullptr) {\n    m_handle = *handle;\n}")
        );
        // Looks the ops up in a registry by type and provider.
        cls.set_function(
            constructor("Runtime")
                .set_template("typename RegistryT, typename StringT")
                .set_param("const RegistryT&", "registry")
                .set_param("const StringT&", "type")
                .set_param("const StringT&", "provider")
                .set_body_line("void* ops = nullptr;")
                .set_body_line(
                    "registry.get_ops()->get(registry.get_handle(), type.get_handle(), "
                    "provider.get_handle(), &ops);"
                )
                .set_body_line("m_ops = static_cast<const OpsStruct*>(ops);")
        );
        cls.set_function(constructor("Runtime")
                             .set_template("typename RegistryT, typename StringT")
                             .set_param("const RegistryT&", "registry")
                             .set_param("const HandleStruct&", "handle")
                             .set_param("const StringT&", "type")
                             .set_param("const StringT&", "provider")
                             .set_initializer("Runtime(registry, type, provider)")
                             .set_body_line("m_handle = handle;"));
        cls.set_function(getter("get_handle", "HandleStruct*", "return &m_handle;"));
        cls.set_function(getter("get_ops", "const OpsStruct*", "return m_ops;"));
        cls.set_function(getter(
                             "operator bool",
                             "",
                             "return m_ops != nullptr && m_handle.plugin_data != nullptr;"
        )
                             .set_explicit());

        cls.set_protected_member_variable("const OpsStruct*", "ops", "nullptr");
        cls.set_protected_member_variable("mutable HandleStruct", "handle", "");

        return cls;
    }

    // `ice::builder::Runtime<Self, Ops, Handle, Dependencies...>`: plugin side of one object.
    static classify::Classify builder_runtime_class()
    {
        classify::Classify cls{"Runtime"};
        cls.set_template(
            "typename Self, typename OpsStruct, typename HandleStruct, typename... Dependencies"
        );

        cls.set_function(constructor("Runtime")
                             .set_explicit()
                             .set_param("const Dependencies*...", "dependencies")
                             .set_initializer(
                                 "m_handle{.plugin_data = static_cast<Self*>(this)}, "
                                 "m_dependencies{dependencies...}"
                             ));
        cls.set_function(
            classify::Function{"Runtime", "", classify::FunctionType::CONSTRUCTOR}
                .set_deleted()
                .set_param("const Runtime&", "")
        );
        cls.set_function(
            classify::Function{"operator=", "Runtime&"}.set_deleted().set_param(
                "const Runtime&",
                ""
            )
        );
        cls.set_function(
            classify::Function{"~Runtime", "", classify::FunctionType::DESTRUCTOR}
                .set_virtual()
                .set_defaulted()
        );
        cls.set_function(getter("from_handle", "Self&", "return *static_cast<Self*>(ctx);")
                             .set_const(false)
                             .set_static()
                             .set_param("void*", "ctx"));
        cls.set_function(
            getter("from_handle", "Self&", "return *static_cast<Self*>(handle->plugin_data);")
                .set_const(false)
                .set_template("typename HandleT")
                .set_static()
                .set_param("HandleT*", "handle")
        );
        cls.set_function(getter("get_vtable", "const OpsStruct&", "return m_vtable;"));
        cls.set_function(getter("get_handle", "const HandleStruct&", "return m_handle;"));
        cls.set_function(
            getter(
                "register_ops",
                "void",
                "registry.register_op(type, provider, const_cast<OpsStruct*>(&m_vtable));"
            )
                .set_template("typename RegistryT, typename StringT")
                .set_param("const RegistryT&", "registry")
                .set_param("const StringT&", "type")
                .set_param("const StringT&", "provider")
        );
        // Sonic view of another vtable's handle, through the ops this object was given.
        cls.set_function(
            getter(
                "wrap",
                "Sonic",
                "return Sonic{std::get<const typename Sonic::Ops*>(m_dependencies), handle};"
            )
                .set_template("typename Sonic, typename HandleT")
                .set_param("std::type_identity<Sonic>", "")
                .set_param("HandleT*", "handle")
        );

        cls.set_protected_member_variable("OpsStruct", "vtable", "");
        cls.set_protected_member_variable("HandleStruct", "handle", "");
        cls.set_member_variable("std::tuple<const Dependencies*...>", "dependencies", "");

        return cls;
    }

    // `ice::sonic::EnumValue<CEnum>`: a C enum value with conversions both ways.
    static classify::Classify enum_value_class()
    {
        classify::Classify cls{"EnumValue"};
        cls.set_template("typename CEnum");

        cls.set_function(constructor("EnumValue").set_constexpr().set_defaulted());
        cls.set_function(constructor("EnumValue")
                             .set_constexpr()
                             .set_param("CEnum", "value")
                             .set_initializer("m_value{value}"));
        cls.set_function(getter("get_value", "CEnum", "return m_value;").set_constexpr());
        cls.set_function(getter("operator CEnum", "", "return m_value;").set_constexpr());

        cls.set_protected_member_variable("CEnum", "value", "");

        return cls;
    }

    // `ice::sonic::StructValue<CStruct>`: a C struct by value, struct_size filled in.
    static classify::Classify struct_value_class()
    {
        classify::Classify cls{"StructValue"};
        cls.set_template("typename CStruct");

        cls.set_function(constructor("StructValue")
                             .set_body_from_raw_string(
                                 "if constexpr (requires(CStruct value) { value.struct_size; }) {\n"
                                 "    m_value.struct_size = sizeof(CStruct);\n"
                                 "}"
                             ));
        cls.set_function(constructor("StructValue")
                             .set_explicit()
                             .set_param("const CStruct&", "value")
                             .set_initializer("m_value{value}"));
        cls.set_function(getter("get_handle", "CStruct*", "return &m_value;").set_const(false));
        cls.set_function(getter("get_handle", "const CStruct*", "return &m_value;"));

        cls.set_protected_member_variable("CStruct", "value", "");

        return cls;
    }

    static classify::Function constructor(std::string name)
    {
        classify::Function function{std::move(name), "", classify::FunctionType::CONSTRUCTOR};
        function.set_noexcept();

        return function;
    }

    // `type name() const noexcept { body }`; a conversion operator when `type` is empty.
    static classify::Function getter(std::string name, std::string type, std::string body)
    {
        const auto KIND =
            type.empty() ? classify::FunctionType::CONVERSION : classify::FunctionType::STANDARD;
        classify::Function function{std::move(name), std::move(type), KIND};
        function.set_const().set_noexcept().set_body_line(std::move(body));

        return function;
    }

    // A base.cppm and a BUILD file for every folder that received a partition or a sub module.
    std::expected<void, std::string> generate_base_modules()
    {
        const auto OUTPUT_ROOT = m_repo_root / m_output_dir;

        for (const auto& [DIRECTORY, CHILDREN]: m_partitions_by_folder) {
            // The output root itself has no module.
            if (DIRECTORY == OUTPUT_ROOT) {
                continue;
            }

            auto module_name = join(DIRECTORY, "_", false);
            if (!module_name) {
                return std::unexpected(std::move(module_name.error()));
            }

            classify::ModuleBuilder base{*module_name};
            (void)base.set_global_fragment(banner("//", "") + '\n')
                .set_raw_import("// clang-format off");

            classify::BuildFile build{*module_name};
            build.set_banner(banner("#", ""));

            for (const std::string& child: CHILDREN) {
                const auto CHILD_PATH = DIRECTORY / child;

                // A child without an entry of its own is a partition file of this leaf module.
                if (!m_partitions_by_folder.contains(CHILD_PATH)) {
                    (void)base.set_raw_import(std::format("export import :{};", child));
                    build.set_partition(std::format("{}.cppm", child));
                    continue;
                }

                auto child_module = join(CHILD_PATH, "_", false);
                auto child_label = bazel_label(CHILD_PATH);
                if (!child_module || !child_label) {
                    return std::unexpected(
                        std::format("cannot name module of {}", CHILD_PATH.string())
                    );
                }

                (void)base.set_raw_import(std::format("export import {};", *child_module));
                build.set_dependency(std::move(*child_label));
            }
            (void)base.set_raw_import("// clang-format on");

            for (const std::string& label: m_dependencies_by_folder[DIRECTORY]) {
                build.set_dependency(label);
            }

            auto written =
                m_writer.write(base.generate_code(), DIRECTORY / "base.cppm", m_repo_root);
            if (!written) {
                return written;
            }

            written = m_writer.write(build.generate_code(), DIRECTORY / "BUILD", m_repo_root);
            if (!written) {
                return written;
            }
        }

        return {};
    }

    // Writes `<directory>/<partition>.cppm` and registers every folder level up to it.
    std::expected<void, std::string> write_partition(
        const std::filesystem::path& directory,
        const std::string& partition,
        const classify::ModuleBuilder& module
    )
    {
        auto written = m_writer.write(
            module.generate_code(),
            directory / std::format("{}.cppm", partition),
            m_repo_root
        );
        if (!written) {
            return written;
        }

        const auto OUTPUT_ROOT = m_repo_root / m_output_dir;
        auto current_node = OUTPUT_ROOT;
        for (const auto& part: std::filesystem::relative(directory, OUTPUT_ROOT)) {
            auto& children = m_partitions_by_folder[current_node];
            if (!std::ranges::contains(children, part.string())) {
                children.push_back(part.string());
            }
            current_node /= part;
        }

        auto& leaf_children = m_partitions_by_folder[current_node];
        if (!std::ranges::contains(leaf_children, partition)) {
            leaf_children.push_back(partition);
        }

        return {};
    }

    // Example: include/c/intern/status.h, sonic -> <output>/include/cc/ice/intern/sonic
    [[nodiscard]] std::filesystem::path
    module_directory(const classify::Classify& model, parser::Mode mode) const
    {
        const bool IS_VTABLE = model.get_kind() == classify::Kind::VTABLE;

        return output_directory(
            std::filesystem::path{model.get_header_path()}.parent_path(),
            IS_VTABLE ? mode : parser::Mode::SONIC
        );
    }

    // Example: sonic -> <output>/include/cc/ice/runtime/sonic
    [[nodiscard]] std::filesystem::path runtime_directory(parser::Mode mode) const
    {
        return output_directory("include/c/runtime", mode);
    }

    [[nodiscard]] std::filesystem::path
    output_directory(const std::filesystem::path& source_directory, parser::Mode mode) const
    {
        auto path = m_repo_root / m_output_dir / source_directory / layer(mode);
        if (m_path_callback) {
            m_path_callback(path);
        }

        return path;
    }

    static std::string_view layer(parser::Mode mode) noexcept
    {
        return mode == parser::Mode::BUILDER ? "builder" : "sonic";
    }

    // Vtable: header stem (`module` -> `module_`). Value: snake_case class name (`pool_id`).
    static std::string partition_name(const classify::Classify& model)
    {
        if (model.get_kind() == classify::Kind::VTABLE) {
            std::string name = std::filesystem::path{model.get_header_path()}.stem().string();

            return name == "module" || name == "import" || name == "export" ? name + '_' : name;
        }

        std::string name;
        for (const char CHARACTER: model.get_class_name()) {
            if (std::isupper(static_cast<unsigned char>(CHARACTER)) != 0 && !name.empty()) {
                name += '_';
            }
            name += static_cast<char>(std::tolower(static_cast<unsigned char>(CHARACTER)));
        }

        return name;
    }

    // Headers a module needs besides its own: dependency vtables (builder) and wrapped values.
    std::set<std::string> dependency_includes(const classify::Classify& model, parser::Mode mode)
    {
        m_scratch_includes = {model.get_header_path()};
        if (model.get_kind() != classify::Kind::VTABLE) {
            return m_scratch_includes;
        }

        const parser::Visitor& visitor = m_parser.get_visitor();
        if (mode == parser::Mode::BUILDER) {
            for (const classify::Classify* dependency: visitor.dependencies_of(model)) {
                m_scratch_includes.insert(dependency->get_header_path());
            }
        }
        for (const classify::Classify* value: visitor.value_dependencies_of(model)) {
            m_scratch_includes.insert(value->get_header_path());
        }

        return m_scratch_includes;
    }

    // Fills the scratch imports and dependencies of one module.
    std::expected<void, std::string> collect_imports(
        const classify::Classify& model,
        parser::Mode mode,
        const std::filesystem::path& directory
    )
    {
        m_scratch_imports.clear();
        m_scratch_dependencies = {c_headers_label(model.get_header_path())};

        const bool IS_VTABLE = model.get_kind() == classify::Kind::VTABLE;
        auto added =
            add_import(runtime_directory(IS_VTABLE ? mode : parser::Mode::SONIC), "", directory);
        if (!added || !IS_VTABLE) {
            return added;
        }

        const parser::Visitor& visitor = m_parser.get_visitor();
        for (const auto& dependencies:
             {visitor.dependencies_of(model), visitor.value_dependencies_of(model)}) {
            for (const classify::Classify* dependency: dependencies) {
                // A sonic class reaches itself without an import.
                if (mode == parser::Mode::SONIC && dependency->get_c_name() == model.get_c_name()) {
                    continue;
                }

                added = add_import(
                    module_directory(*dependency, parser::Mode::SONIC),
                    partition_name(*dependency),
                    directory
                );
                if (!added) {
                    return added;
                }
            }
        }

        return {};
    }

    // `import :partition;` inside the same module, `import module;` plus its Bazel dep outside.
    std::expected<void, std::string> add_import(
        const std::filesystem::path& dependency_directory,
        std::string_view partition,
        const std::filesystem::path& directory
    )
    {
        if (dependency_directory == directory) {
            m_scratch_imports.insert(std::format("import :{};", partition));

            return {};
        }

        auto module_name = join(dependency_directory, "_", false);
        auto label = bazel_label(dependency_directory);
        if (!module_name || !label) {
            return std::unexpected(
                std::format("cannot name module of {}", dependency_directory.string())
            );
        }

        m_scratch_imports.insert(std::format("import {};", *module_name));
        m_scratch_dependencies.insert(std::move(*label));

        return {};
    }

    // Example: include/cc/ice/intern/sonic -> //include/cc/ice/intern/sonic:cc_ice_intern_sonic
    static std::expected<std::string, std::string>
    bazel_label(const std::filesystem::path& directory)
    {
        auto package = join(directory, "/", true);
        auto name = join(directory, "_", false);
        if (!package || !name) {
            return std::unexpected(
                std::format("path has no `include` component: {}", directory.string())
            );
        }

        return std::format("//{}:{}", *package, *name);
    }

    // Path parts from `include` on, joined: …/include/cc/ice/intern/sonic -> cc_ice_intern_sonic
    static std::expected<std::string, std::string>
    join(const std::filesystem::path& directory, std::string_view separator, bool keep_marker)
    {
        auto marker = std::ranges::find(directory, std::filesystem::path{"include"});
        if (marker == directory.end()) {
            return std::unexpected(
                std::format("path has no `include` component: {}", directory.string())
            );
        }
        if (!keep_marker) {
            ++marker;
        }

        std::string joined;
        for (const auto& part: std::ranges::subrange(marker, directory.end())) {
            if (!part.empty()) {
                joined += joined.empty() ? part.string() : std::string{separator} + part.string();
            }
        }

        return joined;
    }

    // Example: include/c/intern/status.h -> //include/c:tf_c_headers
    static std::string c_headers_label(std::string_view header_path)
    {
        std::string package;
        for (const auto& part: std::filesystem::path{header_path} | std::views::take(2)) {
            package += package.empty() ? part.string() : "/" + part.string();
        }

        return std::format("//{}:tf_c_headers", package);
    }

    // Example: // GENERATED by cc_abi_gen from include/c/intern/status.h; run `make gen-cc-abi`.
    [[nodiscard]] std::string banner(std::string_view comment, std::string_view source) const
    {
        return std::format(
            "{0} GENERATED by {1}_abi_gen{2}; run `make gen-{1}-abi`. Do not edit.\n",
            comment,
            m_base_folder,
            source.empty() ? std::string{} : std::format(" from {}", source)
        );
    }

    std::filesystem::path m_repo_root;
    std::filesystem::path m_output_dir;
    std::string m_base_folder;
    std::string m_namespace_name;
    PathCallback m_path_callback;
    parser::Registry m_registry;
    parser::Parser m_parser{"clang++", m_registry};
    Writer m_writer;
    std::map<std::filesystem::path, std::vector<std::string>> m_partitions_by_folder;
    std::map<std::filesystem::path, std::set<std::string>> m_dependencies_by_folder;
    std::set<std::string> m_scratch_imports;
    std::set<std::string> m_scratch_dependencies;
    std::set<std::string> m_scratch_includes;
};

} // namespace cc_abi_gen

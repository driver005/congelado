module;

#include <print>
#include <stdio.h>

export module cc_abi_gen_generator:vtable_emitter;

import std;
import cc_abi_gen_parser;
import cc_abi_gen_writer;
import :helper_formatter;
import :helper_module_naming;
import :helper_types;
import :runtime_spec;
import :runtime_emitter;
import :value_emitter;

export namespace cc_abi_gen::generator::emitter {


class VTableEmitter
{
public:
    VTableEmitter(
        const parser::Registry& registry,
        std::string&& base_folder,
        std::string&& name_space
    ) :
        m_namespace_name{std::move(name_space)},
        m_base_folder{std::move(base_folder)},
        m_registry{registry}
    {
    }

    ~VTableEmitter() = default;
    VTableEmitter(const VTableEmitter&) = delete;
    VTableEmitter& operator=(const VTableEmitter&) = delete;
    VTableEmitter(VTableEmitter&&) = default;
    VTableEmitter& operator=(VTableEmitter&&) = default;

    VTableEmitter& add_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
        return *this;
    }

    VTableEmitter& add_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
        return *this;
    }

    VTableEmitter& add_base_folder(std::string&& base_folder) noexcept
    {
        m_base_folder = std::move(base_folder);
        return *this;
    }

    VTableEmitter& add_file_writer(writer::Writer&& file_writer) noexcept
    {
        m_file_writer = std::move(file_writer);
        return *this;
    }

    VTableEmitter& add_registry(const parser::Registry& registry) noexcept
    {
        m_registry = registry;
        return *this;
    }

    VTableEmitter& add_path_callback(PathCallback&& path_callback) noexcept
    {
        m_path_callback = std::move(path_callback);
        return *this;
    }

    // Unified generate method - uses root for path resolution
    std::expected<void, std::string> generate(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        if (mode == Mode::Both) {
            auto result = generate(root, out_dir, model, Mode::Builder);
            if (!result) {
                return result;
            }

            return generate(root, out_dir, model, Mode::Sonic);
        }

        auto base_path = root / out_dir;

        auto components = extract_path_components(model, mode);
        if (!components) {
            return std::unexpected(components.error());
        }

        auto path = build_output_path(base_path, *components);

        auto module_name = helper::ModuleNaming::module_name(path.parent_path());
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto imports = collect_imports(base_path, model, mode, path.parent_path());
        if (!imports) {
            return std::unexpected(std::move(imports.error()));
        }

        auto rendered = render(base_path, model, mode, *module_name, *imports);
        if (!rendered) {
            return std::unexpected(rendered.error());
        }

        auto write_result = m_file_writer.write(*rendered, path, root);
        if (!write_result) {
            return write_result;
        }

        m_dependencies_by_folder[path.parent_path()].insert(
            m_scratch_dependencies.begin(),
            m_scratch_dependencies.end()
        );


        // Dynamically build the graph folder-by-folder explicitly from the model's structure
        track_partition(base_path, *components);

        return {};
    }

    // Wrapper class of a plain C struct or enum. Always lives in the sonic module of its folder.
    std::expected<void, std::string> generate_value(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::value::Model& model
    )
    {
        auto base_path = root / out_dir;

        auto path = value_path(base_path, model);
        auto module_name = helper::ModuleNaming::module_name(path.parent_path());
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto rendered = ValueEmitter{m_base_folder, m_namespace_name}.render(model, *module_name);
        if (!rendered) {
            return std::unexpected(std::move(rendered.error()));
        }

        auto write_result = m_file_writer.write(*rendered, path, root);
        if (!write_result) {
            return write_result;
        }

        m_dependencies_by_folder[path.parent_path()].insert(
            c_headers_label(model.get_header_path())
        );
        track_partition(
            base_path,
            PathComponents{
                .tier = model.to_tier(),
                .domain = model.get_domain_name(),
                .mode = "sonic",
                .file = model.to_file_name()
            }
        );

        return {};
    }

    std::expected<bool, std::string> check_value(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::value::Model& model
    )
    {
        auto base_path = root / out_dir;

        auto path = value_path(base_path, model);
        auto module_name = helper::ModuleNaming::module_name(path.parent_path());
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto rendered = ValueEmitter{m_base_folder, m_namespace_name}.render(model, *module_name);
        if (!rendered) {
            return std::unexpected(std::move(rendered.error()));
        }

        auto diff_result = m_file_writer.diff(*rendered, path, root);
        if (!diff_result) {
            return std::unexpected(std::move(diff_result.error()));
        }

        if (diff_result->get_identical()) {
            std::println(stderr, "[cc_abi_gen] up to date: {}", path.string());
            return true;
        }

        std::println("--- {} differs ---", path.string());
        std::print("{}", diff_result->get_unified_diff());

        return false;
    }

    std::expected<void, std::string>
    generate_base_modules(std::filesystem::path& root, std::string_view out_dir)
    {
        auto output_root = root / out_dir;

        // Traverse the naturally built directory graph and write a base.cppm and BUILD file for
        // EVERY node
        for (const auto& [dir_path, children]: m_partitions_by_folder) {
            // NOTE: in my current setup the root node has no module
            if (dir_path == output_root) {
                continue;
            }

            auto module_name = helper::ModuleNaming::module_name(dir_path);
            if (!module_name) {
                return std::unexpected(std::move(module_name.error()));
            }

            std::vector<std::string> imports;
            std::vector<std::string> partitions;
            std::vector<std::string> deps;

            for (const auto& child: children) {
                const auto child_path = dir_path / child;

                // A child without an entry of its own is a partition file of this leaf module
                if (!m_partitions_by_folder.contains(child_path)) {
                    imports.push_back(":" + child);
                    partitions.push_back(child);
                    continue;
                }

                auto child_module = helper::ModuleNaming::module_name(child_path);
                if (!child_module) {
                    return std::unexpected(std::move(child_module.error()));
                }

                auto child_label = helper::ModuleNaming::bazel_label(child_path);
                if (!child_label) {
                    return std::unexpected(std::move(child_label.error()));
                }

                imports.push_back(std::move(*child_module));
                deps.push_back(std::move(*child_label));
            }

            if (auto extra = m_dependencies_by_folder.find(dir_path);
                extra != m_dependencies_by_folder.end()) {
                for (const auto& label: extra->second) {
                    if (std::ranges::find(deps, label) == deps.end()) {
                        deps.push_back(label);
                    }
                }
            }

            auto base_rendered = helper::format_base_module(m_base_folder, *module_name, imports);
            if (!base_rendered) {
                return std::unexpected(std::move(base_rendered.error()));
            }

            auto build_rendered = helper::format_build_file(
                m_base_folder,
                m_namespace_name,
                *module_name,
                partitions,
                deps
            );
            if (!build_rendered) {
                return std::unexpected(std::move(build_rendered.error()));
            }

            auto write_result = m_file_writer.write(*base_rendered, dir_path / "base.cppm", root);
            if (!write_result) {
                return write_result;
            }

            auto build_write_result =
                m_file_writer.write(*build_rendered, dir_path / "BUILD", root);
            if (!build_write_result) {
                return build_write_result;
            }
        }

        return {};
    }

    std::expected<bool, std::string> check(
        std::filesystem::path& root,
        std::string_view out_dir,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        if (mode == Mode::Both) {
            auto result = check(root, out_dir, model, Mode::Builder);
            if (!result) {
                return result;
            }

            if (!*result) {
                return false;
            }

            return check(root, out_dir, model, Mode::Sonic);
        }

        auto base_path = root / out_dir;

        auto components = extract_path_components(model, mode);
        if (!components) {
            return std::unexpected(components.error());
        }

        auto real_path = build_output_path(base_path, *components);

        auto module_name = helper::ModuleNaming::module_name(real_path.parent_path());
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto imports = collect_imports(base_path, model, mode, real_path.parent_path());
        if (!imports) {
            return std::unexpected{std::move(imports.error())};
        }

        auto rendered = render(base_path, model, mode, *module_name, *imports);
        if (!rendered) {
            return std::unexpected{std::move(rendered.error())};
        }

        auto diff_result = m_file_writer.diff(*rendered, real_path, root);
        if (!diff_result) {
            return std::unexpected{std::move(diff_result.error())};
        }

        if (diff_result->get_identical()) {
            std::println(stderr, "[cc_abi_gen] up to date: {}", real_path.string());
        } else {
            std::println("--- {} differs ---", real_path.string());
            std::print("{}", diff_result->get_unified_diff());
            return false;
        }

        return true;
    }

    std::expected<void, std::string>
    generate_runtime(std::filesystem::path& root, std::string_view out_dir)
    {
        auto base_path = root / out_dir;

        auto prepared = prepare_runtime(base_path);
        if (!prepared) {
            return prepared;
        }

        for (const auto& spec: RuntimeEmitter::get_specs()) {
            auto target = runtime_target(base_path, spec);
            if (!target) {
                return std::unexpected(std::move(target.error()));
            }

            auto module_name = helper::ModuleNaming::module_name(*target);
            if (!module_name) {
                return std::unexpected(std::move(module_name.error()));
            }

            auto rendered = m_runtime_emitter.render(spec, *module_name);
            if (!rendered) {
                return std::unexpected(std::move(rendered.error()));
            }

            auto path = *target / std::format("{}.cppm", spec.get_partition());
            auto write_result = m_file_writer.write(*rendered, path, root);
            if (!write_result) {
                return write_result;
            }

            auto& children = m_partitions_by_folder[*target];
            if (std::ranges::find(children, std::string{spec.get_partition()}) == children.end()) {
                children.emplace_back(spec.get_partition());
            }

            auto runtime_dir = runtime_directory(base_path);
            if (!runtime_dir) {
                return std::unexpected(std::move(runtime_dir.error()));
            }

            auto anchor = m_registry.get().find(std::string{spec.get_anchor_struct()});

            m_scratch_imports.clear();
            m_scratch_dependencies.clear();
            m_scratch_dependencies.insert(c_headers_label(anchor->get()));
            auto added =
                add_import(*runtime_dir, std::string{RuntimeEmitter::k_runtime_partition}, *target);
            if (!added) {
                return added;
            }
            m_dependencies_by_folder[*target].insert(
                m_scratch_dependencies.begin(),
                m_scratch_dependencies.end()
            );
        }

        return {};
    }

    std::expected<bool, std::string>
    check_runtime(std::filesystem::path& root, std::string_view out_dir)
    {
        auto base_path = root / out_dir;

        auto prepared = prepare_runtime(base_path);
        if (!prepared) {
            return std::unexpected(std::move(prepared.error()));
        }

        bool identical = true;
        for (const auto& spec: RuntimeEmitter::get_specs()) {
            auto target = runtime_target(base_path, spec);
            if (!target) {
                return std::unexpected(std::move(target.error()));
            }

            auto module_name = helper::ModuleNaming::module_name(*target);
            if (!module_name) {
                return std::unexpected(std::move(module_name.error()));
            }

            auto rendered = m_runtime_emitter.render(spec, *module_name);
            if (!rendered) {
                return std::unexpected(std::move(rendered.error()));
            }

            auto path = *target / std::format("{}.cppm", spec.get_partition());
            auto diff_result = m_file_writer.diff(*rendered, path, root);
            if (!diff_result) {
                return std::unexpected(std::move(diff_result.error()));
            }

            if (diff_result->get_identical()) {
                std::println(stderr, "[cc_abi_gen] up to date: {}", path.string());
            } else {
                std::println("--- {} differs ---", path.string());
                std::print("{}", diff_result->get_unified_diff());
                identical = false;
            }
        }

        return identical;
    }

    void set_path_callback(PathCallback&& path_callback) noexcept
    {
        m_path_callback = std::move(path_callback);
    }

    void set_writer(std::string&& writer) noexcept
    {
        m_writer = std::move(writer);
    }

    void set_namespace_name(std::string&& namespace_name) noexcept
    {
        m_namespace_name = std::move(namespace_name);
    }

    void set_base_folder(std::string&& base_folder) noexcept
    {
        m_base_folder = std::move(base_folder);
    }

    void set_file_writer(writer::Writer&& file_writer) noexcept
    {
        m_file_writer = std::move(file_writer);
    }

    void set_registry(const parser::Registry& registry)
    {
        m_registry = std::cref(registry);
    }

    const std::string& get_writer() noexcept
    {
        return m_writer;
    }

    const std::string& get_namespace_name() noexcept
    {
        return m_namespace_name;
    }

    const std::string& get_base_folder() noexcept
    {
        return m_base_folder;
    }

    const writer::Writer& get_file_writer() noexcept
    {
        return m_file_writer;
    }

    const parser::Registry& get_registry() noexcept
    {
        return m_registry;
    }

    const PathCallback& get_output_path_callback() const noexcept
    {
        return m_path_callback;
    }

private:
    void track_partition(const std::filesystem::path& output_root, const PathComponents& components)
    {
        // Translate the strings directly into a path to leverage slash-by-slash iteration
        std::filesystem::path base_path =
            std::filesystem::path(components.tier) / components.domain / components.mode;

        if (m_path_callback) {
            m_path_callback(base_path);
        }

        std::filesystem::path current_node = output_root;

        // Traverse each slash component and register it in the parent's vector
        for (const auto& part: base_path) {
            std::string child_name = part.string();
            auto& children = m_partitions_by_folder[current_node];

            if (std::ranges::find(children, child_name) == children.end()) {
                children.push_back(child_name);
            }

            current_node /= part; // Step into the next folder level
        }

        // At the leaf node, register the actual file (partition)
        std::string leaf_str = components.file;
        auto& leaf_children = m_partitions_by_folder[current_node];
        if (std::ranges::find(leaf_children, leaf_str) == leaf_children.end()) {
            leaf_children.push_back(std::move(leaf_str));
        }
    }

    std::expected<std::string, std::string> render(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const Mode& mode,
        std::string_view module_name,
        std::string_view imports
    )
    {
        m_writer.clear();

        auto handle_name = c_handle_name(model);
        if (handle_name.empty()) {
            return std::unexpected(
                std::format("No C handle struct found for '{}'", model.get_struct_name())
            );
        }

        auto partition = model.to_file_name();
        if (!partition.has_value()) {
            return std::unexpected("Failed to get partition name");
        }

        auto members = member_dependencies(model, mode);
        if (!members) {
            return std::unexpected(std::move(members.error()));
        }

        auto string_type = runtime_type_name(RuntimeEmitter::k_string_struct);
        if (!string_type) {
            return std::unexpected(std::move(string_type.error()));
        }

        auto registry_type = registry_type_name(model, mode);
        if (!registry_type) {
            return std::unexpected(std::move(registry_type.error()));
        }

        auto includes = dependency_includes(model, *members);
        if (!includes) {
            return std::unexpected(std::move(includes.error()));
        }

        auto header_result = helper::format_header(
            to_gen_target(mode),
            m_base_folder,
            model.get_domain_name(),
            model.get_header_path(),
            model.get_class_name(),
            m_namespace_name,
            root,
            module_name,
            model.get_struct_name(),
            *partition,
            handle_name,
            imports,
            *members,
            *string_type,
            *includes,
            *registry_type
        );
        if (!header_result) {
            return std::unexpected(header_result.error());
        }

        m_writer += *header_result;

        for (const parser::slot::Slot& slot: model.get_slots()) {
            if (mode == Mode::Builder && slot.get_name() == RuntimeEmitter::k_create_slot) {
                continue;
            }

            auto method = write_method(root, slot, mode);
            if (!method.has_value()) {
                return std::unexpected(method.error());
            }
        }

        if (mode == Mode::Builder) {
            auto accessor = write_vtable_accessor(root, model, mode);
            if (!accessor.has_value()) {
                return std::unexpected(accessor.error());
            }
        } else if (mode != Mode::Sonic) {
            return std::unexpected(std::format("Invalid mode for render function: {}", mode));
        }

        auto footer_result = helper::format_footer(
            to_gen_target(mode),
            model.get_struct_name(),
            handle_name,
            *members,
            root,
            *string_type,
            *registry_type
        );
        if (!footer_result) {
            return std::unexpected(footer_result.error());
        }
        m_writer += *footer_result;

        return m_writer;
    }

    static helper::GenTarget to_gen_target(Mode mode) noexcept
    {
        switch (mode) {
            case Mode::Builder:
                return helper::GenTarget::Builder;
            case Mode::Sonic:
                return helper::GenTarget::Sonic;
            case Mode::Both:
                return helper::GenTarget::Builder; // unused for Both
        }
    }

    std::expected<PathComponents, std::string>
    extract_path_components(const parser::vtable::Model& model, const Mode& mode) const
    {
        auto tier = model.to_tier();
        if (!tier.has_value()) {
            return std::unexpected{tier.error()};
        }

        std::string domain{model.get_domain_name()};
        auto file = model.to_file_name();
        if (!file.has_value()) {
            return std::unexpected{std::move(file.error())};
        }


        std::string mode_str;
        if (mode == emitter::Mode::Sonic) {
            mode_str = "sonic";
        } else if (mode == emitter::Mode::Builder) {
            mode_str = "builder";
        } else {
            return std::unexpected{
                std::format("Mode not supported in extract_path_components `{}`", mode)
            };
        }

        return PathComponents{
            .tier = *std::move(tier),
            .domain = std::move(domain),
            .mode = std::move(mode_str),
            .file = *std::move(file)
        };
    }

    std::filesystem::path
    build_output_path(const std::filesystem::path& root, const PathComponents& components) const
    {
        auto calculated_path = root / components.tier / components.domain / components.mode /
                               (components.file + ".cppm");

        if (m_path_callback) {
            m_path_callback(calculated_path);
        }
        return calculated_path;
    }

    std::expected<void, std::string>
    write_method(std::filesystem::path& root, const parser::slot::Slot& slot, const Mode& mode)
    {
        auto ms_result = helper::format_method_signature(slot.get_name(), mode == Mode::Builder);
        if (!ms_result) {
            return std::unexpected(ms_result.error());
        }
        m_writer += *ms_result;

        auto parameters_list = write_cpp_parameter_list(slot, mode == Mode::Builder);
        if (!parameters_list.has_value()) {
            return parameters_list;
        }

        if (mode == Mode::Builder) {
            m_writer += helper::format_virtual_method_end(root);

            return {};
        }

        auto mbs_result = helper::format_method_body_start(slot.get_name(), preamble(slot, mode));
        if (!mbs_result) {
            return std::unexpected(mbs_result.error());
        }
        m_writer += *mbs_result;

        if (!slot.extract_parameters().empty()) {
            m_writer += ", ";
        }

        auto call_arguments = write_call_arguments(slot, mode);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        m_writer += helper::format_method_body_end(epilogue(slot, mode));

        return {};
    }

    std::expected<void, std::string>
    write_cpp_parameter_list(const parser::slot::Slot& slot, bool is_builder_list)
    {
        bool first = true;
        for (const parser::helper::Parameter& parameter: slot.extract_parameters()) {
            const auto use = classify(slot, parameter);
            if (use == ParameterUse::ArrayLength) {
                continue;
            }
            if (!first) {
                m_writer += ", ";
            }
            first = false;

            if (use == ParameterUse::ArrayIn || use == ParameterUse::ArrayOut) {
                // The caller (sonic) holds the array in a vector, the implementer (builder) gets
                // a span over the host's array.
                const bool input = use == ParameterUse::ArrayIn;
                auto array_result = helper::format_parameter(
                    is_builder_list ? std::format(
                                          "std::span<{}{}>",
                                          input ? "const " : "",
                                          element_type(parameter, true)
                                      )
                                    : std::format(
                                          "{}std::vector<{}>&",
                                          input ? "const " : "",
                                          element_type(parameter, false)
                                      ),
                    parameter.get_name()
                );
                if (!array_result) {
                    return std::unexpected(array_result.error());
                }
                m_writer += *array_result;
                continue;
            }

            if (use == ParameterUse::Enum || use == ParameterUse::EnumOut ||
                use == ParameterUse::StructPointer) {
                const auto value = find_value(parameter);
                std::string type = value_type(value->get());
                if (use == ParameterUse::StructPointer) {
                    type = (parameter.is_const_pointee() ? "const " : "") + type + "*";
                } else if (use == ParameterUse::EnumOut) {
                    type += "&";
                }
                auto value_result = helper::format_parameter(type, parameter.get_name());
                if (!value_result) {
                    return std::unexpected(value_result.error());
                }
                m_writer += *value_result;
                continue;
            }

            if (use == ParameterUse::ScalarReference) {
                auto reference_result = helper::format_parameter(
                    parameter.get_value_type() + "&",
                    parameter.get_name()
                );
                if (!reference_result) {
                    return std::unexpected(reference_result.error());
                }
                m_writer += *reference_result;
                continue;
            }

            auto model = parameter.has_pointee()
                             ? m_registry.get().find(parameter.get_registry_key())
                             : std::nullopt;

            if (!model.has_value()) {
                auto param_result =
                    helper::format_parameter(parameter.get_qualified_type(), parameter.get_name());
                if (!param_result) {
                    return std::unexpected(param_result.error());
                }
                m_writer += *param_result;
                continue;
            }

            auto param_result = helper::format_parameter(
                model->get().to_pointee_type(m_namespace_name),
                parameter.get_name()
            );
            if (!param_result) {
                return std::unexpected(param_result.error());
            }
            m_writer += *param_result;
        }

        return {};
    }

    // A slot with a length or capacity parameter hands out arrays, which stay plain pointers.
    bool has_array(const parser::slot::Slot& slot) const noexcept
    {
        static constexpr std::array<std::string_view, 5>
            k_length_words{"num", "count", "capacity", "max", "len"};

        for (const parser::helper::Parameter& candidate: slot.extract_parameters()) {
            if (candidate.get_type().contains('*') || candidate.get_name().starts_with("out_")) {
                continue;
            }
            for (const std::string_view word: k_length_words) {
                if (candidate.get_name().contains(word)) {
                    return true;
                }
            }
        }

        return false;
    }

    std::optional<std::reference_wrapper<const parser::value::Model>>
    find_value(const parser::helper::Parameter& parameter) const
    {
        return m_registry.get().find_value(parameter.get_value_key());
    }

    // The integer parameter that carries the element count of the array starting at `parameter`.
    const parser::helper::Parameter* length_of(
        const parser::slot::Slot& slot,
        const parser::helper::Parameter& parameter
    ) const noexcept
    {
        static constexpr std::array<std::string_view, 5>
            k_length_words{"num", "count", "capacity", "max", "len"};

        if (!parameter.is_element_pointer()) {
            return nullptr;
        }

        const auto parameters = slot.extract_parameters();
        const auto index = static_cast<std::size_t>(&parameter - parameters.data());
        // A string list hands out char** next to its lengths; those stay plain pointers.
        if (index > 0 && parameters[index - 1].get_type().ends_with("**")) {
            return nullptr;
        }

        if (index + 1 < parameters.size()) {
            const parser::helper::Parameter& candidate = parameters[index + 1];
            if (candidate.is_integer() && !candidate.get_name().starts_with("out_")) {
                for (const std::string_view word: k_length_words) {
                    if (candidate.get_name().contains(word)) {
                        return &candidate;
                    }
                }
            }
        }

        // An array right behind a length shares it: tensor_view(dims, num_dims, strides, ...).
        if (index > 0 && parameters[index - 1].is_integer() && index > 1 &&
            length_of(slot, parameters[index - 2]) == &parameters[index - 1]) {
            return &parameters[index - 1];
        }

        return nullptr;
    }

    bool is_array_length(
        const parser::slot::Slot& slot,
        const parser::helper::Parameter& parameter
    ) const noexcept
    {
        const auto parameters = slot.extract_parameters();
        const auto index = static_cast<std::size_t>(&parameter - parameters.data());

        return index > 0 && length_of(slot, parameters[index - 1]) == &parameter;
    }

    // The array parameter whose element count `length` carries.
    const parser::helper::Parameter&
    array_of(const parser::slot::Slot& slot, const parser::helper::Parameter& length) const noexcept
    {
        const auto parameters = slot.extract_parameters();

        return parameters[static_cast<std::size_t>(&length - parameters.data()) - 1];
    }

    // Element type of an array parameter. Example: std::int64_t, ice::sonic::DataTypeEnum
    // `raw` asks for the C element type, which a span over the host's array needs.
    std::string element_type(const parser::helper::Parameter& parameter, bool raw) const
    {
        const auto value = find_value(parameter);
        if (!raw && value.has_value() && value->get().get_kind() == parser::value::Kind::Enum) {
            return value_type(value->get());
        }

        return parameter.get_value_type();
    }

    // True when the elements are a wrapped enum, which the C side holds as the raw C enum.
    bool has_wrapped_elements(const parser::helper::Parameter& parameter) const
    {
        const auto value = find_value(parameter);

        return value.has_value() && value->get().get_kind() == parser::value::Kind::Enum;
    }

    // The argument that stands for an array on either side of the C boundary: a span over the
    // host's array for the implementer (builder), the vector's data() for the caller (sonic).
    std::string array_argument(
        const parser::slot::Slot& slot,
        const parser::helper::Parameter& parameter,
        ParameterUse use,
        const Mode& mode
    ) const
    {
        const std::string name{parameter.get_name()};
        if (mode == Mode::Builder) {
            return std::format(
                "std::span<{}{}>{{{}, static_cast<std::size_t>({})}}",
                use == ParameterUse::ArrayIn ? "const " : "",
                element_type(parameter, true),
                name,
                length_of(slot, parameter)->get_name()
            );
        }

        return (has_wrapped_elements(parameter) ? "raw_" : "") + name + ".data()";
    }

    // The sonic argument for an element count: the smallest size of the vectors that share it.
    std::string
    length_argument(const parser::slot::Slot& slot, const parser::helper::Parameter& length) const
    {
        std::vector<std::string> sizes;
        for (const parser::helper::Parameter& candidate: slot.extract_parameters()) {
            if (length_of(slot, candidate) == &length) {
                sizes.push_back(std::format("{}.size()", candidate.get_name()));
            }
        }

        std::string size = sizes.front();
        if (sizes.size() > 1) {
            size = "std::min({";
            for (std::size_t index = 0; index < sizes.size(); ++index) {
                size += (index == 0 ? "" : ", ") + sizes[index];
            }
            size += "})";
        }

        return std::format("static_cast<{}>({})", length.get_type(), size);
    }

    ParameterUse
    classify(const parser::slot::Slot& slot, const parser::helper::Parameter& parameter) const
    {
        if (length_of(slot, parameter) != nullptr) {
            return parameter.is_const_pointee() ? ParameterUse::ArrayIn : ParameterUse::ArrayOut;
        }
        if (is_array_length(slot, parameter)) {
            return ParameterUse::ArrayLength;
        }

        const auto value = find_value(parameter);
        if (value.has_value()) {
            if (value->get().get_kind() == parser::value::Kind::Enum) {
                if (!parameter.is_pointer()) {
                    return ParameterUse::Enum;
                }
                if (parameter.get_name().starts_with("out_") && !parameter.is_const_pointee() &&
                    !has_array(slot)) {
                    return ParameterUse::EnumOut;
                }

                return ParameterUse::Plain;
            }
            if (parameter.is_pointer() && !has_array(slot)) {
                return ParameterUse::StructPointer;
            }

            return ParameterUse::Plain;
        }

        if (parameter.is_scalar_out() && !has_array(slot)) {
            return ParameterUse::ScalarReference;
        }

        return ParameterUse::Plain;
    }

    // Example: ice::sonic::PoolId
    std::string value_type(const parser::value::Model& value) const
    {
        return std::format("{}::sonic::{}", m_namespace_name, value.get_class_name());
    }

    // The C++ argument that stands for a wrapped parameter, on either side of the C boundary.
    std::string value_argument(
        ParameterUse use,
        const parser::value::Model& value,
        const parser::helper::Parameter& parameter,
        const Mode& mode
    ) const
    {
        const std::string name{parameter.get_name()};
        if (mode == Mode::Builder) {
            switch (use) {
                case ParameterUse::Enum:
                    return std::format("{}{{{}}}", value_type(value), name);
                case ParameterUse::EnumOut:
                    return "wrapped_" + name;
                default:
                    return std::format("wrapped_{0} ? &*wrapped_{0} : nullptr", name);
            }
        }

        switch (use) {
            case ParameterUse::Enum:
                return name + ".get_value()";
            case ParameterUse::EnumOut:
                return "&raw_" + name;
            default:
                return std::format("{0} == nullptr ? nullptr : {0}->get_handle()", name);
        }
    }

    // Sonic: unwrap enum elements into a raw C vector. The builder reads the host array in place.
    std::string array_preamble(
        const parser::slot::Slot& slot,
        const parser::helper::Parameter& parameter,
        ParameterUse use,
        const Mode& mode
    ) const
    {
        const std::string name{parameter.get_name()};
        if (mode == Mode::Builder) {
            return {};
        }
        if (!has_wrapped_elements(parameter)) {
            return {};
        }

        const auto value = find_value(parameter);
        std::string text = std::format(
            "                std::vector<::{0}> raw_{1}({1}.size());\n",
            value->get().get_c_name(),
            name
        );
        if (use == ParameterUse::ArrayIn) {
            text += std::format(
                "                for (std::size_t index = 0; index < {0}.size(); ++index) {{\n"
                "                    raw_{0}[index] = {0}[index].get_value();\n"
                "                }}\n",
                name
            );
        }

        return text;
    }

    // Sonic: wrap raw enum elements again. The builder writes the host array in place.
    std::string array_epilogue(
        const parser::slot::Slot& slot,
        const parser::helper::Parameter& parameter,
        const Mode& mode
    ) const
    {
        const std::string name{parameter.get_name()};
        const bool wrapped = has_wrapped_elements(parameter);
        if (mode == Mode::Builder) {
            return {};
        }
        if (!wrapped) {
            return {};
        }

        return std::format(
            "                for (std::size_t index = 0; index < {0}.size(); ++index) {{\n"
            "                    {0}[index] = {1}{{raw_{0}[index]}};\n"
            "                }}\n",
            name,
            element_type(parameter, false)
        );
    }

    // Statements that run before the C call (sonic) or the virtual call (builder).
    std::string preamble(const parser::slot::Slot& slot, const Mode& mode) const
    {
        std::string text;
        for (const parser::helper::Parameter& parameter: slot.extract_parameters()) {
            const auto use = classify(slot, parameter);
            if (use == ParameterUse::ArrayIn || use == ParameterUse::ArrayOut) {
                text += array_preamble(slot, parameter, use, mode);
                continue;
            }
            if (use != ParameterUse::EnumOut && use != ParameterUse::StructPointer) {
                continue;
            }

            const auto value = find_value(parameter);
            const std::string name{parameter.get_name()};
            if (mode == Mode::Builder && use == ParameterUse::EnumOut) {
                text += std::format(
                    "                {} wrapped_{}{{*{}}};\n",
                    value_type(value->get()),
                    name,
                    name
                );
            } else if (mode == Mode::Builder) {
                text += std::format(
                    "                std::optional<{0}> wrapped_{1};\n"
                    "                if ({1} != nullptr) {{\n"
                    "                    wrapped_{1}.emplace(*{1});\n"
                    "                }}\n",
                    value_type(value->get()),
                    name
                );
            } else if (use == ParameterUse::EnumOut) {
                text += std::format(
                    "                ::{} raw_{}{{}};\n",
                    value->get().get_c_name(),
                    name
                );
            }
        }

        return text;
    }

    // Statements that run after the call and copy results back through the wrapped parameters.
    std::string epilogue(const parser::slot::Slot& slot, const Mode& mode) const
    {
        std::string text;
        for (const parser::helper::Parameter& parameter: slot.extract_parameters()) {
            const auto use = classify(slot, parameter);
            const std::string name{parameter.get_name()};
            if (use == ParameterUse::ArrayOut) {
                text += array_epilogue(slot, parameter, mode);
            } else if (use == ParameterUse::EnumOut && mode == Mode::Builder) {
                text += std::format("                *{0} = wrapped_{0}.get_value();\n", name);
            } else if (use == ParameterUse::EnumOut) {
                const auto value = find_value(parameter);
                text += std::format(
                    "                {} = {}{{raw_{}}};\n",
                    name,
                    value_type(value->get()),
                    name
                );
            } else if (
                use == ParameterUse::StructPointer && mode == Mode::Builder &&
                !parameter.is_const_pointee()
            ) {
                text += std::format(
                    "                if ({0} != nullptr) {{\n"
                    "                    *{0} = *wrapped_{0}->get_handle();\n"
                    "                }}\n",
                    name
                );
            }
        }

        return text;
    }

    // Value classes the parameters of a model depend on, once each.
    std::vector<const parser::value::Model*>
    value_dependencies_of(const parser::vtable::Model& model) const
    {
        std::map<std::string, const parser::value::Model*> unique;
        for (const parser::slot::Slot& slot: model.get_slots()) {
            for (const parser::helper::Parameter& parameter: slot.extract_parameters()) {
                const auto use = classify(slot, parameter);
                if (use == ParameterUse::Plain || use == ParameterUse::ScalarReference ||
                    use == ParameterUse::ArrayLength) {
                    continue;
                }
                const auto value = find_value(parameter);
                if (value.has_value()) {
                    unique.emplace(value->get().get_c_name(), &value->get());
                }
            }
        }

        std::vector<const parser::value::Model*> result;
        for (const auto& [c_name, value]: unique) {
            result.push_back(value);
        }

        return result;
    }

    std::expected<void, std::string> add_value_import(
        const std::filesystem::path& base_path,
        const parser::value::Model& value,
        const std::filesystem::path& module_dir
    )
    {
        const auto directory = value_path(base_path, value).parent_path();

        return add_import(directory, value.to_file_name(), module_dir);
    }

    std::expected<void, std::string>
    write_call_arguments(const parser::slot::Slot& slot, const Mode& mode)
    {
        auto middle = slot.extract_parameters();

        bool first = true;
        for (const parser::helper::Parameter& parameter: middle) {
            const auto use = classify(slot, parameter);
            if (use == ParameterUse::ArrayLength && mode == Mode::Builder) {
                continue;
            }
            if (!first) {
                m_writer += ", ";
            }
            first = false;

            if (use == ParameterUse::ArrayLength) {
                m_writer += length_argument(slot, parameter);
                continue;
            }
            if (use == ParameterUse::ArrayIn || use == ParameterUse::ArrayOut) {
                m_writer += array_argument(slot, parameter, use, mode);
                continue;
            }
            if (use == ParameterUse::ScalarReference) {
                m_writer += mode == Mode::Builder ? "*" : "&";
                m_writer += parameter.get_name();
                continue;
            }
            if (use != ParameterUse::Plain) {
                const auto value = find_value(parameter);
                m_writer += value_argument(use, value->get(), parameter, mode);
                continue;
            }

            auto model = parameter.has_pointee()
                             ? m_registry.get().find(parameter.get_registry_key())
                             : std::nullopt;

            if (!model.has_value()) {
                m_writer += parameter.get_name();
                continue;
            }

            if (mode == Mode::Builder) {
                m_writer += model->get().wrape_type(m_namespace_name, parameter.get_name());
            } else if (mode == Mode::Sonic) {
                m_writer += model->get().unwrape_type(parameter.get_name());
            } else {
                return std::unexpected(
                    std::format("Invalid mode for write_call_arguments function: {}", mode)
                );
            }
        }

        return {};
    }

    std::expected<void, std::string> write_vtable_accessor(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const Mode& mode
    )
    {
        auto vtas_result = helper::format_vtable_accessor_start(
            model.get_struct_name(),
            std::format(
                "TF_OFFSET_OF_END(::{}, {})",
                model.get_struct_name(),
                model.get_slots().back().get_name()
            ),
            c_handle_name(model),
            root
        );
        if (!vtas_result) {
            return std::unexpected(vtas_result.error());
        }
        m_writer += *vtas_result;

        for (const parser::slot::Slot& slot: model.get_slots()) {
            auto field = write_vtable_field(root, model, slot, mode);
            if (!field.has_value()) {
                return field;
            }
        }

        m_writer += helper::format_vtable_accessor_end(root);

        return {};
    }

    std::expected<void, std::string> write_vtable_field(
        std::filesystem::path& root,
        const parser::vtable::Model& model,
        const parser::slot::Slot& slot,
        const Mode& mode
    )
    {
        if (slot.get_name() == RuntimeEmitter::k_create_slot) {
            m_writer += "\n            .create = create,\n";

            return {};
        }

        auto vtfgs_result = helper::format_vtable_field_generic_start(slot.get_name());
        if (!vtfgs_result) {
            return std::unexpected(vtfgs_result.error());
        }
        m_writer += *vtfgs_result;

        write_c_parameter_list(slot.get_parameters());

        auto vtfm_result = helper::format_vtable_field_middle(
            model.get_class_name(),
            slot.get_name(),
            self_parameter_name(slot),
            preamble(slot, mode)
        );
        if (!vtfm_result) {
            return std::unexpected(vtfm_result.error());
        }
        m_writer += *vtfm_result;

        auto call_arguments = write_call_arguments(slot, mode);
        if (!call_arguments.has_value()) {
            return call_arguments;
        }

        m_writer += helper::format_vtable_field_end(epilogue(slot, mode));

        return {};
    }

    std::string_view self_parameter_name(const parser::slot::Slot& slot) const noexcept
    {
        auto parameters = slot.get_parameters();
        return parameters.empty() ? std::string_view{"plugin_context"}
                                  : parameters.front().get_name();
    }

    // Example: "TFGrapplerConfigsOps" -> "TFGrapplerConfigs" when a slot takes that handle
    std::string c_handle_name(const parser::vtable::Model& model) const
    {
        std::string_view struct_name = model.get_struct_name();
        if (!struct_name.ends_with("Ops")) {
            return {};
        }

        std::string handle_name{struct_name.substr(0, struct_name.size() - 3)};
        for (const parser::slot::Slot& slot: model.get_slots()) {
            auto parameters = slot.get_parameters();
            if (!parameters.empty() && parameters.front().get_pointee_name() == handle_name) {
                return handle_name;
            }
        }

        return {};
    }

    void write_c_parameter_list(std::span<const parser::helper::Parameter> parameters)
    {
        for (auto&& [index, parameter]: parameters | std::views::enumerate) {
            if (index != 0) {
                m_writer += ", ";
            }

            auto param_result =
                helper::format_parameter(parameter.get_qualified_type(), parameter.get_name());
            if (!param_result) {
                // This is a void function, but format_parameter returns expected
                // In practice this shouldn't fail for simple parameters
                m_writer += "/* error */";
                continue;
            }
            m_writer += *param_result;
        }
    }

    std::expected<void, std::string> add_import(
        const std::filesystem::path& dependency_directory,
        std::string_view partition,
        const std::filesystem::path& module_directory
    )
    {
        if (dependency_directory == module_directory) {
            m_scratch_imports.insert(std::format("import :{};", partition));

            return {};
        }

        auto module_name = helper::ModuleNaming::module_name(dependency_directory);
        if (!module_name) {
            return std::unexpected(std::move(module_name.error()));
        }

        auto label = helper::ModuleNaming::bazel_label(dependency_directory);
        if (!label) {
            return std::unexpected(std::move(label.error()));
        }

        m_scratch_imports.insert(std::format("import {};", *module_name));
        m_scratch_dependencies.insert(std::move(*label));

        return {};
    }

    std::expected<std::filesystem::path, std::string> module_directory(
        const std::filesystem::path& base_path,
        const parser::vtable::Model& model,
        const Mode& mode
    ) const
    {
        auto components = extract_path_components(model, mode);
        if (!components) {
            return std::unexpected(std::move(components.error()));
        }

        return build_output_path(base_path, *components).parent_path();
    }

    std::expected<std::filesystem::path, std::string>
    runtime_directory(const std::filesystem::path& base_path) const
    {
        auto anchor = m_registry.get().find(std::string{RuntimeEmitter::k_string_struct});
        if (!anchor.has_value()) {
            return std::unexpected(
                std::format(
                    "Runtime anchor '{}' is not part of the parsed headers",
                    RuntimeEmitter::k_string_struct
                )
            );
        }

        return module_directory(base_path, anchor->get(), Mode::Sonic);
    }

    std::expected<void, std::string> add_model_import(
        const std::filesystem::path& base_path,
        const parser::vtable::Model& dependency,
        const parser::vtable::Model& model,
        const Mode& mode,
        const std::filesystem::path& module_dir
    )
    {
        if (mode == Mode::Sonic && dependency.get_struct_name() == model.get_struct_name()) {
            return {};
        }

        auto dependency_directory = module_directory(base_path, dependency, Mode::Sonic);
        if (!dependency_directory) {
            return std::unexpected(std::move(dependency_directory.error()));
        }

        auto partition = dependency.to_file_name();
        if (!partition) {
            return std::unexpected(std::move(partition.error()));
        }

        return add_import(*dependency_directory, *partition, module_dir);
    }

    std::expected<std::string, std::string> dependency_includes(
        const parser::vtable::Model& model,
        std::span<const helper::DependencyInfo> members
    ) const
    {
        m_scratch_includes.clear();

        for (const helper::DependencyInfo& member: members) {
            m_scratch_includes.insert(member.get_header_path());
        }
        if (auto registry_model =
                m_registry.get().find(std::string{RuntimeEmitter::k_registry_struct})) {
            m_scratch_includes.insert(std::string{registry_model->get().get_header_path()});
        }
        for (const parser::value::Model* value: value_dependencies_of(model)) {
            m_scratch_includes.insert(value->get_header_path());
        }
        m_scratch_includes.erase(std::string{model.get_header_path()});

        std::string joined;
        for (const auto& header: m_scratch_includes) {
            joined += std::format("#include \"{}\"\n", header);
        }

        return joined;
    }

    static bool is_runtime_root(const parser::vtable::Model& model) noexcept
    {
        return model.get_struct_name() == RuntimeEmitter::k_string_struct ||
               model.get_struct_name() == RuntimeEmitter::k_registry_struct;
    }

    // Empty for the String and registry roots in sonic mode: they cannot look themselves up.
    std::expected<std::string, std::string>
    registry_type_name(const parser::vtable::Model& model, const Mode& mode) const
    {
        if (mode == Mode::Sonic && is_runtime_root(model)) {
            return std::string{};
        }

        return runtime_type_name(RuntimeEmitter::k_registry_struct);
    }

    std::expected<std::string, std::string> runtime_type_name(std::string_view struct_name) const
    {
        auto model = m_registry.get().find(std::string{struct_name});
        if (!model.has_value()) {
            return std::unexpected(
                std::format("Runtime anchor '{}' is not part of the parsed headers", struct_name)
            );
        }

        return model->get().to_sonic_type(m_namespace_name);
    }

    std::expected<std::vector<helper::DependencyInfo>, std::string>
    member_dependencies(const parser::vtable::Model& model, const Mode& mode) const
    {
        std::vector<helper::DependencyInfo> infos;
        if (mode != Mode::Builder) {
            return infos;
        }

        auto dependencies = dependencies_of(model);
        if (!dependencies) {
            return std::unexpected(std::move(dependencies.error()));
        }

        std::map<std::string, const parser::vtable::Model*> unique;
        for (const parser::vtable::Model* dependency: *dependencies) {
            unique.emplace(dependency->get_struct_name(), dependency);
        }

        for (const auto& [struct_name, member]: unique) {
            auto handle_name = c_handle_name(*member);
            if (handle_name.empty()) {
                return std::unexpected(
                    std::format("No C handle struct found for '{}'", member->get_struct_name())
                );
            }

            infos.emplace_back(
                member->to_sonic_type(m_namespace_name),
                std::string{member->get_struct_name()},
                std::string{member->get_class_name()},
                std::move(handle_name),
                std::string{member->get_header_path()}
            );
        }

        return infos;
    }

    std::expected<std::vector<const parser::vtable::Model*>, std::string>
    dependencies_of(const parser::vtable::Model& model) const
    {
        std::vector<const parser::vtable::Model*> dependencies;

        for (const parser::slot::Slot& slot: model.get_slots()) {
            for (const parser::helper::Parameter& parameter: slot.extract_parameters()) {
                auto dependency = parameter.has_pointee()
                                      ? m_registry.get().find(parameter.get_registry_key())
                                      : std::nullopt;
                if (dependency.has_value()) {
                    dependencies.push_back(&dependency->get());
                }
            }
        }

        return dependencies;
    }

    std::filesystem::path
    value_path(const std::filesystem::path& base_path, const parser::value::Model& model) const
    {
        auto path = base_path / model.to_tier() / model.get_domain_name() / "sonic" /
                    (model.to_file_name() + ".cppm");
        if (m_path_callback) {
            m_path_callback(path);
        }

        return path;
    }

    static std::string c_headers_label(const parser::vtable::Model& model)
    {
        return c_headers_label(model.get_header_path());
    }

    static std::string c_headers_label(std::string_view header_path)
    {
        const std::filesystem::path header{header_path};

        std::string package;
        for (const auto& part: header | std::views::take(2)) {
            package += package.empty() ? part.string() : "/" + part.string();
        }

        return std::format("//{}:tf_c_headers", package);
    }

    std::expected<std::string, std::string> collect_imports(
        const std::filesystem::path& base_path,
        const parser::vtable::Model& model,
        const Mode& mode,
        const std::filesystem::path& module_dir
    )
    {
        m_scratch_imports.clear();
        m_scratch_dependencies.clear();
        m_scratch_dependencies.insert(c_headers_label(model));

        auto runtime_dir = runtime_directory(base_path);
        if (!runtime_dir) {
            return std::unexpected(std::move(runtime_dir.error()));
        }

        auto runtime_import =
            add_import(*runtime_dir, std::string{RuntimeEmitter::k_runtime_partition}, module_dir);
        if (!runtime_import) {
            return std::unexpected(std::move(runtime_import.error()));
        }

        auto dependencies = dependencies_of(model);
        if (!dependencies) {
            return std::unexpected(std::move(dependencies.error()));
        }

        for (const parser::vtable::Model* dependency: *dependencies) {
            auto added = add_model_import(base_path, *dependency, model, mode, module_dir);
            if (!added) {
                return std::unexpected(std::move(added.error()));
            }
        }

        for (const parser::value::Model* value: value_dependencies_of(model)) {
            auto added = add_value_import(base_path, *value, module_dir);
            if (!added) {
                return std::unexpected(std::move(added.error()));
            }
        }

        auto string_model = m_registry.get().find(std::string{RuntimeEmitter::k_string_struct});
        if (!string_model.has_value()) {
            return std::unexpected(
                std::format(
                    "Runtime anchor '{}' is not part of the parsed headers",
                    RuntimeEmitter::k_string_struct
                )
            );
        }

        auto string_import =
            add_model_import(base_path, string_model->get(), model, mode, module_dir);
        if (!string_import) {
            return std::unexpected(std::move(string_import.error()));
        }

        if (!(mode == Mode::Sonic && is_runtime_root(model))) {
            auto registry_model =
                m_registry.get().find(std::string{RuntimeEmitter::k_registry_struct});
            if (!registry_model.has_value()) {
                return std::unexpected("Runtime registry anchor is not part of the parsed headers");
            }

            auto registry_import =
                add_model_import(base_path, registry_model->get(), model, mode, module_dir);
            if (!registry_import) {
                return std::unexpected(std::move(registry_import.error()));
            }
        }

        std::string joined;
        for (const auto& import_line: m_scratch_imports) {
            joined += import_line;
            joined += '\n';
        }

        return joined;
    }

    std::expected<void, std::string> prepare_runtime(const std::filesystem::path& base_path)
    {
        m_runtime_emitter.clear_values();

        auto runtime_dir = runtime_directory(base_path);
        if (!runtime_dir) {
            return std::unexpected(std::move(runtime_dir.error()));
        }

        auto runtime_module = helper::ModuleNaming::module_name(*runtime_dir);
        if (!runtime_module) {
            return std::unexpected(std::move(runtime_module.error()));
        }

        auto string_model = m_registry.get().find(std::string{RuntimeEmitter::k_string_struct});
        auto registry_model = m_registry.get().find(std::string{RuntimeEmitter::k_registry_struct});
        if (!string_model.has_value() || !registry_model.has_value()) {
            return std::unexpected(
                "Runtime String or registry anchor is not part of the parsed headers"
            );
        }

        m_runtime_emitter.add_value("namespace_name", std::string{m_namespace_name})
            .add_value("runtime_module", std::move(*runtime_module))
            .add_value("string_handle", c_handle_name(string_model->get()))
            .add_value("registry_header", std::string{registry_model->get().get_header_path()})
            .add_value("registry_struct", std::string{registry_model->get().get_struct_name()})
            .add_value("registry_handle", c_handle_name(registry_model->get()));

        return {};
    }

    std::expected<std::filesystem::path, std::string>
    runtime_target(const std::filesystem::path& base_path, const RuntimeSpec& spec) const
    {
        auto anchor = m_registry.get().find(std::string{spec.get_anchor_struct()});
        if (!anchor.has_value()) {
            return std::unexpected(
                std::format(
                    "Runtime anchor '{}' is not part of the parsed headers",
                    spec.get_anchor_struct()
                )
            );
        }

        return module_directory(base_path, anchor->get(), Mode::Sonic);
    }

    std::string m_writer;
    std::string m_namespace_name;
    std::string m_base_folder;
    writer::Writer m_file_writer;
    std::reference_wrapper<const parser::Registry> m_registry;
    PathCallback m_path_callback;
    std::map<std::filesystem::path, std::vector<std::string>> m_partitions_by_folder;
    std::map<std::filesystem::path, std::set<std::string>> m_dependencies_by_folder;
    std::set<std::string> m_scratch_imports;
    std::set<std::string> m_scratch_dependencies;
    mutable std::set<std::string> m_scratch_includes;
    RuntimeEmitter m_runtime_emitter;
};

} // namespace cc_abi_gen::generator::emitter

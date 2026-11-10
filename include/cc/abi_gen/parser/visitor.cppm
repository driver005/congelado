module;

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/SourceManager.h>

export module cc_abi_gen_parser:visitor;

import std;
import cc_abi_gen_classify;
import :parameter_text;
import :utils;

export namespace cc_abi_gen::parser {

// Every parsed C declaration by C name; sorted, so generated listings are stable.
using Registry = std::map<std::string, classify::Classify>;

// One generation path: clang AST -> model (pass 1), model + registry -> class (pass 2).
class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
    Visitor(Registry& registry, std::string namespace_name) :
        m_registry{registry},
        m_namespace_name{std::move(namespace_name)}
    {
    }

    // Pass 1 over one header's translation unit.
    void parse(
        clang::TranslationUnitDecl* unit,
        const clang::SourceManager& source_manager,
        std::string header_path
    )
    {
        m_source_manager = source_manager;
        m_header_path = std::move(header_path);
        TraverseDecl(unit);
    }

    // NOLINTNEXTLINE(readability-identifier-naming,bugprone-derived-method-shadowing-base-method)
    bool VisitRecordDecl(clang::RecordDecl* record_decl)
    {
        if (is_own_top_level(record_decl)) {
            add(read_record(record_decl));
        }

        return true;
    }

    // NOLINTNEXTLINE(readability-identifier-naming,bugprone-derived-method-shadowing-base-method)
    bool VisitEnumDecl(clang::EnumDecl* enum_decl)
    {
        if (is_own_top_level(enum_decl)) {
            add(read_enum(enum_decl));
        }

        return true;
    }

    // Pass 2. Needs the whole header tree in the registry: handles and wrapped values.
    std::expected<classify::Classify, std::string>
    build(const classify::Classify& model, Mode mode) const
    {
        if (model.get_kind() != classify::Kind::VTABLE) {
            return model;
        }

        const std::string HANDLE = c_handle_name(model);
        if (HANDLE.empty()) {
            return std::unexpected(
                std::format("No C handle struct found for '{}'", model.get_c_name())
            );
        }

        classify::Classify cls{model.get_class_name()};
        cls.set_public_raw("using Runtime::Runtime;\n");
        if (mode == Mode::SONIC) {
            cls.set_base_class(
                std::format(
                    "{}::sonic::Runtime<::{}, ::{}>",
                    m_namespace_name,
                    model.get_c_name(),
                    HANDLE
                )
            );
            for (const classify::Function& slot: model.get_functions()) {
                classify::Function method{slot.get_name(), "void"};
                method.set_const().set_noexcept();
                method.set_body_from_raw_string(translate_slot(slot, Mode::SONIC, method));
                cls.set_function(std::move(method));
            }

            return cls;
        }

        // The builder base holds the ops of every vtable a slot takes, to wrap their handles.
        std::string dependencies;
        for (const classify::Classify* dependency: dependencies_of(model)) {
            if (c_handle_name(*dependency).empty()) {
                return std::unexpected(
                    std::format("No C handle struct found for '{}'", dependency->get_c_name())
                );
            }
            dependencies += std::format(", ::{}", dependency->get_c_name());
        }
        cls.set_base_class(
            std::format(
                "{}::builder::Runtime<{}, ::{}, ::{}{}>",
                m_namespace_name,
                model.get_class_name(),
                model.get_c_name(),
                HANDLE,
                dependencies
            )
        );

        // One pure virtual per slot, and one lambda per slot that forwards the C call to it.
        classify::BodyBuilder body;
        body.begin_vtable(
            model.get_c_name(),
            std::format(
                "TF_OFFSET_OF_END(::{}, {})",
                model.get_c_name(),
                model.get_functions().back().get_name()
            )
        );
        for (const classify::Function& slot: model.get_functions()) {
            // The host constructs the object through `create`, so it has no virtual to call.
            if (slot.get_name() == "create") {
                body.add_entry(".create = create");
                continue;
            }

            classify::Function method{slot.get_name(), "void"};
            method.set_virtual().set_noexcept().set_pure_virtual();
            const std::string CALL = translate_slot(slot, Mode::BUILDER, method);
            cls.set_function(std::move(method));

            body.begin_slot(slot.get_name());
            for (const classify::Param& parameter: slot.get_parameters()) {
                body.add_param(
                    classify::Param{parameter.get_qualified_type(), parameter.get_name()}
                );
            }
            body.add_self_handle(model.get_class_name(), self_parameter_name(slot));
            body.add_raw(CALL);
            body.end_slot();
        }
        body.end_vtable();

        auto text = body.take();
        if (!text) {
            return std::unexpected(std::move(text.error()));
        }
        cls.set_function(
            classify::Function{"get_generic_vtable", "void"}
                .set_noexcept()
                .set_param(std::format("void (*)(::{}*)", HANDLE), "create")
                .set_body_from_raw_string(*text)
        );

        return cls;
    }

    // Every vtable this one takes as a slot parameter, once per C struct.
    [[nodiscard]] std::vector<const classify::Classify*>
    dependencies_of(const classify::Classify& model) const
    {
        return collect(
            model,
            [this](const classify::Param& parameter)
            {
                return find_vtable(parameter);
            }
        );
    }

    // Every plain struct or enum a slot parameter of this vtable wraps, once per C name.
    [[nodiscard]] std::vector<const classify::Classify*>
    value_dependencies_of(const classify::Classify& model) const
    {
        return collect(
            model,
            [this](const classify::Param& parameter)
            {
                return find_value(parameter);
            }
        );
    }

    void set_namespace_name(std::string namespace_name)
    {
        m_namespace_name = std::move(namespace_name);
    }

private:
    using Lookup = std::optional<std::reference_wrapper<const classify::Classify>>;

    // Declared in this header at file (or extern "C") scope, not nested in another record.
    bool is_own_top_level(const clang::Decl* decl) const
    {
        return m_source_manager.has_value() &&
               m_source_manager->get().isInMainFile(decl->getLocation()) &&
               decl->getDeclContext()->getRedeclContext()->isFileContext();
    }

    void add(std::expected<classify::Classify, std::string> model)
    {
        if (!model) {
            return;
        }

        std::string key = model->get_c_name();
        m_registry.get().try_emplace(std::move(key), std::move(*model));
    }

    // One loop over the fields decides the shape: vtable, handle (skipped) or plain struct.
    std::expected<classify::Classify, std::string> read_record(clang::RecordDecl* record_decl)
    {
        if (!is_named_definition(record_decl) || record_decl->field_empty()) {
            return std::unexpected("Record is not a named complete definition with fields");
        }

        const std::string C_NAME = record_decl->getNameAsString();
        classify::Classify vtable = start_class(C_NAME, classify::Kind::VTABLE);
        bool has_slot = false;
        bool has_plugin_data = false;
        m_scratch_fields.clear();

        for (clang::FieldDecl* field: record_decl->fields()) {
            const std::string NAME = field->getNameAsString();
            if (field->getType()->isFunctionPointerType()) {
                if (auto slot = read_slot(field)) {
                    vtable.set_function(std::move(*slot));
                    has_slot = true;
                }
            } else if (NAME == "plugin_data") {
                has_plugin_data = true;
            } else if (
                NAME != "struct_size" && NAME != "ext" && !NAME.empty() &&
                !has_anonymous_type(field->getType())
            ) {
                m_scratch_fields.push_back(field);
            }
        }

        if (has_slot) {
            if ((*record_decl->field_begin())->getNameAsString() != "struct_size") {
                return std::unexpected("Record has slots but no leading struct_size field");
            }

            return vtable;
        }
        if (has_plugin_data || !record_decl->isStruct()) {
            return std::unexpected("Record is a handle or not a struct");
        }

        return read_struct(C_NAME);
    }

    // Sonic wrapper of a plain C struct over m_scratch_fields; StructValue holds the C value.
    classify::Classify read_struct(const std::string& c_name)
    {
        classify::Classify cls = start_class(c_name, classify::Kind::STRUCT);
        const std::string CLASS_NAME = cls.get_class_name();
        cls.set_base_class(std::format("{}::sonic::StructValue<::{}>", m_namespace_name, c_name))
            .set_public_raw("using StructValue::StructValue;\n");

        // Every field a plain value: the struct can be built from its fields in order.
        const bool ALL_SCALAR = !m_scratch_fields.empty() &&
                                std::ranges::none_of(
                                    m_scratch_fields,
                                    [](const clang::FieldDecl* field)
                                    {
                                        return field_extent(field) != 0 || is_record(field);
                                    }
                                );
        if (ALL_SCALAR) {
            cls.set_function(constructor(CLASS_NAME).set_defaulted());

            classify::Function field_ctor = constructor(CLASS_NAME);
            field_ctor.set_explicit(m_scratch_fields.size() == 1);
            for (const clang::FieldDecl* field: m_scratch_fields) {
                const std::string NAME = field->getNameAsString();
                field_ctor.set_param(field_type(field), NAME);
                field_ctor.set_body_line(std::format("m_value.{0} = {0};", NAME));
            }
            cls.set_function(std::move(field_ctor));
        }

        for (const clang::FieldDecl* field: m_scratch_fields) {
            add_field_accessors(cls, field);
        }

        return cls;
    }

    // Sonic wrapper of a C enum: nested `Value` enum on top of EnumValue.
    std::expected<classify::Classify, std::string> read_enum(clang::EnumDecl* enum_decl) const
    {
        if (!is_named_definition(enum_decl)) {
            return std::unexpected("Enum is not a named complete definition");
        }

        const std::string C_NAME = enum_decl->getNameAsString();
        classify::Classify cls = start_class(C_NAME, classify::Kind::ENUM);
        cls.set_base_class(std::format("{}::sonic::EnumValue<::{}>", m_namespace_name, C_NAME))
            .set_public_raw(value_enum(enum_decl, C_NAME) + "\nusing EnumValue::EnumValue;\n");
        cls.set_function(constructor(cls.get_class_name()).set_constexpr().set_defaulted());
        cls.set_function(
            constructor(cls.get_class_name())
                .set_constexpr()
                .set_param("Value", "value")
                .set_initializer(std::format("EnumValue{{static_cast<::{}>(value)}}", C_NAME))
        );

        return cls;
    }

    // Class named after the C declaration, tagged with where it came from.
    [[nodiscard]] classify::Classify
    start_class(const std::string& c_name, classify::Kind kind) const
    {
        classify::Classify cls{class_name(c_name)};
        cls.set_c_name(c_name).set_header_path(m_header_path).set_kind(kind);

        return cls;
    }

    // Adds the C++ parameters of `slot` to `method`, returns the call with its before/after code.
    std::string
    translate_slot(const classify::Function& slot, Mode mode, classify::Function& method) const
    {
        const bool BUILDER = mode == Mode::BUILDER;
        std::string before;
        std::string arguments;
        std::string after;

        for (const classify::Param& parameter: slot_arguments(slot)) {
            const ParameterText TEXT = translate_parameter(slot, parameter, BUILDER);
            if (!TEXT.get_type().empty()) {
                method.set_param(TEXT.get_type(), parameter.get_name());
            }
            if (!TEXT.get_argument().empty()) {
                arguments += arguments.empty() ? TEXT.get_argument() : ", " + TEXT.get_argument();
            }
            before += TEXT.get_before();
            after += TEXT.get_after();
        }

        const std::string CALL = BUILDER ? std::format("self.{}({});\n", slot.get_name(), arguments)
                                         : std::format(
                                               "m_ops->{}(get_handle(){}{});\n",
                                               slot.get_name(),
                                               arguments.empty() ? "" : ", ",
                                               arguments
                                           );

        return before + CALL + after;
    }

    // The first kind of parameter that fits; a plain handle or value is the fallback.
    [[nodiscard]] ParameterText translate_parameter(
        const classify::Function& slot,
        const classify::Param& parameter,
        bool builder
    ) const
    {
        if (auto text = translate_array(slot, parameter, builder)) {
            return *text;
        }
        if (auto text = translate_enum(slot, parameter, builder)) {
            return *text;
        }
        if (auto text = translate_struct(slot, parameter, builder)) {
            return *text;
        }
        if (auto text = translate_scalar(slot, parameter, builder)) {
            return *text;
        }

        return translate_handle(parameter, builder);
    }

    // Array pointers and their length: host span (builder), unwrapped vector (sonic).
    [[nodiscard]] std::optional<ParameterText> translate_array(
        const classify::Function& slot,
        const classify::Param& parameter,
        bool builder
    ) const
    {
        const std::string& name = parameter.get_name();
        ParameterText text;
        if (is_array_length(slot, parameter)) {
            text.set_argument(builder ? "" : length_argument(slot, parameter));

            return text;
        }

        const classify::Param* length = length_of(slot, parameter);
        if (length == nullptr) {
            return std::nullopt;
        }

        const bool INPUT = parameter.is_const_pointee();
        const std::string_view CONSTNESS = INPUT ? "const " : "";
        const std::string RAW = parameter.get_value_type();
        if (builder) {
            text.set_type(std::format("std::span<{}{}>", CONSTNESS, RAW));
            text.set_argument(
                std::format(
                    "std::span<{}{}>{{{}, static_cast<std::size_t>({})}}",
                    CONSTNESS,
                    RAW,
                    name,
                    length->get_name()
                )
            );

            return text;
        }

        const Lookup VALUE = find_value(parameter);
        const bool IS_ENUM = VALUE.has_value() && VALUE->get().get_kind() == classify::Kind::ENUM;
        const std::string WRAPPER = IS_ENUM ? value_type(VALUE->get()) : RAW;
        text.set_type(std::format("{}std::vector<{}>&", CONSTNESS, WRAPPER));
        text.set_argument((IS_ENUM ? "raw_" : "") + name + ".data()");
        if (IS_ENUM) {
            text.set_before(
                std::format(
                    "std::vector<::{0}> raw_{1}({1}.size());\n",
                    VALUE->get().get_c_name(),
                    name
                )
            );
            const std::string LOOP = std::format(
                "for (std::size_t index = 0; index < {0}.size(); ++index) {{\n    {1};\n}}\n",
                name,
                INPUT ? std::format("raw_{0}[index] = {0}[index].get_value()", name)
                      : std::format("{0}[index] = {1}{{raw_{0}[index]}}", name, WRAPPER)
            );
            if (INPUT) {
                text.add_before(LOOP);
            } else {
                text.add_after(LOOP);
            }
        }

        return text;
    }

    // An enum by value, or an `out_*` pointer to one, as its sonic wrapper.
    [[nodiscard]] std::optional<ParameterText> translate_enum(
        const classify::Function& slot,
        const classify::Param& parameter,
        bool builder
    ) const
    {
        const Lookup VALUE = find_value(parameter);
        if (!VALUE.has_value() || VALUE->get().get_kind() != classify::Kind::ENUM) {
            return std::nullopt;
        }

        const std::string& name = parameter.get_name();
        const std::string WRAPPER = value_type(VALUE->get());
        ParameterText text;
        text.set_type(WRAPPER);
        if (!parameter.is_pointer()) {
            text.set_argument(
                builder ? std::format("{}{{{}}}", WRAPPER, name) : name + ".get_value()"
            );

            return text;
        }

        if (!name.starts_with("out_") || parameter.is_const_pointee() || has_array(slot)) {
            return std::nullopt;
        }

        text.set_type(text.get_type() + '&');
        if (builder) {
            text.set_argument("wrapped_" + name);
            text.set_before(std::format("{} wrapped_{}{{*{}}};\n", WRAPPER, name, name));
            text.set_after(std::format("*{0} = wrapped_{0}.get_value();\n", name));
        } else {
            text.set_argument("&raw_" + name);
            text.set_before(std::format("::{} raw_{}{{}};\n", VALUE->get().get_c_name(), name));
            text.set_after(std::format("{} = {}{{raw_{}}};\n", name, WRAPPER, name));
        }

        return text;
    }

    // A pointer to a plain C struct as a pointer to its sonic wrapper.
    [[nodiscard]] std::optional<ParameterText> translate_struct(
        const classify::Function& slot,
        const classify::Param& parameter,
        bool builder
    ) const
    {
        const Lookup VALUE = find_value(parameter);
        if (!VALUE.has_value() || VALUE->get().get_kind() == classify::Kind::ENUM ||
            !parameter.is_pointer() || has_array(slot)) {
            return std::nullopt;
        }

        const std::string& name = parameter.get_name();
        const std::string WRAPPER = value_type(VALUE->get());
        ParameterText text;
        text.set_type((parameter.is_const_pointee() ? "const " : "") + WRAPPER + "*");
        if (!builder) {
            text.set_argument(std::format("{0} == nullptr ? nullptr : {0}->get_handle()", name));

            return text;
        }

        text.set_argument(std::format("wrapped_{0} ? &*wrapped_{0} : nullptr", name));
        text.set_before(
            std::format(
                "std::optional<{0}> wrapped_{1};\n"
                "if ({1} != nullptr) {{\n    wrapped_{1}.emplace(*{1});\n}}\n",
                WRAPPER,
                name
            )
        );
        if (!parameter.is_const_pointee()) {
            text.set_after(
                std::format(
                    "if ({0} != nullptr) {{\n    *{0} = *wrapped_{0}->get_handle();\n}}\n",
                    name
                )
            );
        }

        return text;
    }

    // An `out_*` pointer to a number, as a reference.
    [[nodiscard]] std::optional<ParameterText> translate_scalar(
        const classify::Function& slot,
        const classify::Param& parameter,
        bool builder
    ) const
    {
        if (find_value(parameter).has_value() || !parameter.is_scalar_out() || has_array(slot)) {
            return std::nullopt;
        }

        ParameterText text;
        text.set_type(parameter.get_value_type() + "&");
        text.set_argument((builder ? "*" : "&") + parameter.get_name());

        return text;
    }

    // A handle of another vtable travels as its sonic class; anything else as written.
    [[nodiscard]] ParameterText
    translate_handle(const classify::Param& parameter, bool builder) const
    {
        const std::string& name = parameter.get_name();
        const Lookup VTABLE = find_vtable(parameter);
        ParameterText text;
        if (!VTABLE.has_value()) {
            text.set_type(parameter.get_qualified_type());
            text.set_argument(name);

            return text;
        }

        const std::string TYPE = value_type(VTABLE->get());
        text.set_type(std::format("const {} &", TYPE));
        text.set_argument(
            builder ? std::format("self.wrap(std::type_identity<{}>{{}}, {})", TYPE, name)
                    : name + ".get_handle()"
        );

        return text;
    }

    // Every registry entry `lookup` finds for a slot parameter of `model`, once per C name.
    template<typename LookupFunction>
    [[nodiscard]] std::vector<const classify::Classify*>
    collect(const classify::Classify& model, LookupFunction lookup) const
    {
        std::map<std::string, const classify::Classify*> unique;
        for (const classify::Function& slot: model.get_functions()) {
            for (const classify::Param& parameter: slot_arguments(slot)) {
                if (const Lookup FOUND = lookup(parameter)) {
                    unique.emplace(FOUND->get().get_c_name(), &FOUND->get());
                }
            }
        }

        return unique | std::views::values | std::ranges::to<std::vector>();
    }

    // The vtable a pointer parameter names through its handle.
    [[nodiscard]] Lookup find_vtable(const classify::Param& parameter) const
    {
        if (!parameter.has_pointee()) {
            return std::nullopt;
        }

        return find(parameter.get_registry_key());
    }

    // The plain struct or enum a parameter names, never a vtable.
    [[nodiscard]] Lookup find_value(const classify::Param& parameter) const
    {
        const Lookup MODEL = find(parameter.get_value_key());
        if (!MODEL.has_value() || MODEL->get().get_kind() == classify::Kind::VTABLE) {
            return std::nullopt;
        }

        return MODEL;
    }

    [[nodiscard]] Lookup find(const std::string& c_name) const
    {
        const auto FOUND = m_registry.get().find(c_name);
        if (FOUND == m_registry.get().end()) {
            return std::nullopt;
        }

        return std::cref(FOUND->second);
    }

    // Example: ice::sonic::PoolId, ice::sonic::String
    [[nodiscard]] std::string value_type(const classify::Classify& model) const
    {
        return std::format("{}::sonic::{}", m_namespace_name, model.get_class_name());
    }

    std::reference_wrapper<Registry> m_registry;
    std::string m_namespace_name;
    std::optional<std::reference_wrapper<const clang::SourceManager>> m_source_manager;
    std::string m_header_path;
    std::vector<const clang::FieldDecl*> m_scratch_fields;
};

} // namespace cc_abi_gen::parser

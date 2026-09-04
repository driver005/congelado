export module cc_abi_gen_generator:builder_emitter;

import std;
import cc_abi_gen_parser;
import :type_registry;
import :slot_classifier;

export namespace cc_abi_gen {

// Renders the builder-tier module (include/cc/abi/builder/<domain>/<domain>.cppm): an abstract
// interface plus a get_generic_vtable() that materializes the flat C vtable from lambdas.
// Everything is appended straight into m_writer's one continuous stream — no fragment strings
// are ever assembled first and interpolated in afterward, comma-joined lists included.
class BuilderEmitter
{
public:
    explicit BuilderEmitter(TypeRegistry &type_registry) noexcept : m_classifier(type_registry)
    {
    }

    std::string render(const VtableModel &model)
    {
        m_writer.clear();

        m_writer += std::format(
            R"cpp(module;

#include "c/extern/{0}/{0}.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_{0};

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {{

class {1}
{{
public:
static {1}* create(void* ctx) noexcept {{ return static_cast<{1}*>(ctx); }}

virtual ~{1}() = default;

)cpp",
            model.m_domain_name,
            model.m_class_name
        );

        for (const VtableSlot &slot : model.m_slots) {

            if (m_classifier.is_destroy(slot) || m_classifier.is_get_name(slot)) {

                continue;
            }

            write_virtual_method(slot);
        }

        m_writer += "\nvirtual ice::String get_name() const noexcept = 0;\n\n";
        write_vtable_accessor(model);
        m_writer += "};\n\n} // namespace ice::builder\n";

        return m_writer;
    }

private:
    void write_virtual_method(const VtableSlot &slot)
    {
        m_writer += std::format(
            "[[nodiscard]] virtual std::expected<void, ice::Status> {}(", slot.m_name
        );
        write_cpp_parameter_list(m_classifier.middle_parameters(slot));
        m_writer += ") noexcept = 0;\n";
    }

    void write_cpp_parameter_list(std::span<const Parameter> parameters)
    {
        for (std::size_t index = 0; index < parameters.size(); ++index) {

            if (index != 0) {

                m_writer += ", ";
            }
            m_writer += std::format(
                "{} {}", m_classifier.cpp_parameter_type(parameters[index]), parameters[index].m_name
            );
        }
    }

    void write_vtable_accessor(const VtableModel &model)
    {
        m_writer += std::format(
            "static {0}* get_generic_vtable()\n{{\nstatic {0} vtable = {{\n.struct_size = {1},\n",
            model.m_struct_name,
            model.m_struct_size_macro
        );

        for (const VtableSlot &slot : model.m_slots) {

            write_vtable_field(model, slot);
        }

        m_writer += "};\nreturn &vtable;\n}\n";
    }

    void write_vtable_field(const VtableModel &model, const VtableSlot &slot)
    {
        if (m_classifier.is_destroy(slot)) {

            m_writer += std::format(
                ".{0} = [](void* plugin_context) noexcept {{ delete {1}::create(plugin_context); "
                "}},\n",
                slot.m_name,
                model.m_class_name
            );

            return;
        }

        if (m_classifier.is_get_name(slot)) {

            m_writer += std::format(
                ".{0} = [](void* plugin_context, TF_String* out) noexcept {{ auto* self = "
                "{1}::create(plugin_context); auto name = self->get_name(); name.to_c(out); }},\n",
                slot.m_name,
                model.m_class_name
            );

            return;
        }

        // Bind an intermediate `self` rather than chaining `Class::create(plugin_context)->method(
        // ...)` directly — the chained form reads fine short, but clang-format wraps it onto a
        // second line via the `->` once the argument list pushes it past the column limit, which
        // is exactly the split-call style this generator otherwise normalizes away.
        // "[]" (empty capture) is a literal, plain square brackets — std::format only treats
        // curly braces specially, so no escaping/placeholder is needed for it. Split across three
        // appends, with the C parameter list and the call arguments written directly into
        // m_writer in between — one continuous stream, no intermediate joined-list string.
        m_writer += std::format(".{} = [](", slot.m_name);
        write_c_parameter_list(slot.m_parameters);
        m_writer += std::format(
            R"cpp() noexcept {{
auto* self = {}::create(plugin_context);
auto res = self->{}()cpp",
            model.m_class_name,
            slot.m_name
        );
        write_call_arguments(slot);
        m_writer += std::format(
            R"cpp();
if (!res) {{
res.error().to_c({});
}}
}},
)cpp",
            status_parameter_name(slot)
        );
    }

    void write_c_parameter_list(std::span<const Parameter> parameters)
    {
        for (std::size_t index = 0; index < parameters.size(); ++index) {

            if (index != 0) {

                m_writer += ", ";
            }
            m_writer += std::format("{} {}", parameters[index].m_type, parameters[index].m_name);
        }
    }

    void write_call_arguments(const VtableSlot &slot)
    {
        std::span<const Parameter> middle = m_classifier.middle_parameters(slot);
        for (std::size_t index = 0; index < middle.size(); ++index) {

            if (index != 0) {

                m_writer += ", ";
            }

            m_writer += m_classifier.wrap_argument(middle[index].m_name, middle[index].m_type);
        }
    }

    std::string status_parameter_name(const VtableSlot &slot)
    {
        return slot.m_parameters.empty() ? "status" : slot.m_parameters.back().m_name;
    }

    SlotClassifier m_classifier;
    std::string m_writer;
};

} // namespace cc_abi_gen

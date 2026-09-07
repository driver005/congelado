export module cc_abi_gen_generator:helper_formater;

import std;

export namespace cc_abi_gen::helper {

enum class GenTarget
{
    Builder,
    Sonic
};

inline std::string format_header(
    GenTarget target,
    std::string_view domain_name,
    std::string_view cc_class_name,
    std::string_view namespace_name,
    std::string_view c_struct_name = ""
)
{
    struct Config
    {
        std::string_view target_name;
        std::string_view extra_includes;
        std::string_view extra_imports;
        std::string inheritance;
        std::string class_body;
    };

    const Config config = [&]() -> Config
    {
        if (target == GenTarget::Builder) {
            return {
                .target_name = "builder",
                .extra_includes = R"(
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
                )",
                .extra_imports = "",
                .inheritance = "",
                .class_body = std::format(
                    R"(
                        static {0}* create(void* ctx) noexcept
                        {{
                            return static_cast<{0}*>(ctx);
                        }}

                        virtual ~{0}() = default;
                    )",
                    cc_class_name
                )
            };
        }

        return {
            .target_name = "sonic",
            .extra_includes = "",
            .extra_imports = R"cpp(import cc_abi_sonic_registration;
            )cpp",
            .inheritance = std::format(
                " : public {}::sonic::Runtime<{}, {}>",
                namespace_name,
                cc_class_name,
                c_struct_name
            ),
            .class_body = std::format(
                R"(
                        explicit {0}({1}* ops, void* plugin_context) noexcept 
                            : Runtime(ops, plugin_context)
                        {{
                        }}

                        static constexpr std::string_view domain_name = "{2}";
                )",
                cc_class_name,
                c_struct_name,
                domain_name
            )
        };
    }();

    return std::format(
        R"(module;

#include "c/extern/{0}/{0}.h"
{1}
export module cc_abi_{2}_{0};

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
{3}
export namespace {7}::{2} {{

class {4}{5}
{{
public:
{6}
)",
        // End of string
        domain_name,
        config.extra_includes,
        config.target_name,
        config.extra_imports,
        cc_class_name,
        config.inheritance,
        config.class_body,
        namespace_name
    );
}

inline std::string format_footer(GenTarget target, std::string_view namespace_name)
{
    const std::string_view target_name = (target == GenTarget::Builder) ? "builder" : "sonic";

    const std::string extra_methods = (target == GenTarget::Builder)
                                          ? ""
                                          : std::format(
                                                R"({0}::String get_name() const noexcept
                                                {{
                                                    {0}::String out;
                                                    m_ops->get_name(get_handle(), out.get_handle());
                                                    return out;
                                                }}
                                                )",
                                                namespace_name
                                            );

    return std::format(
        R"(
            {0}
        }};

        }} // namespace {2}::{1}
        )",
        extra_methods,
        target_name,
        namespace_name
    );
}

inline std::string format_parameter(std::string_view type, std::string_view name)
{
    return std::format("{} {}", type, name);
}

inline std::string format_method_signature(
    std::string_view method_name,
    std::string_view namespace_name,
    bool is_virtual = false
)
{
    return std::format(
        "[[nodiscard]] {}std::expected<void, {}::Status> {}(",
        is_virtual ? "virtual " : "",
        namespace_name,
        method_name
    );
}

inline std::string format_get_name_decl(std::string_view namespace_name)
{
    return std::format(
        R"cpp(virtual {}
              ::String get_name() const noexcept = 0;
        )cpp",
        namespace_name
    );
}

constexpr std::string_view format_virtual_method_end()
{
    return R"cpp(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ) noexcept = 0;
    )cpp";
}

inline std::string
format_vtable_accessor_start(std::string_view c_struct_name, std::string_view struct_size_macro)
{
    return std::format(
        R"(
            static {0}* get_generic_vtable()
            {{
                static {0} vtable = {{
                    .struct_size = {1},
        )",
        c_struct_name,
        struct_size_macro
    );
}

constexpr std::string_view format_vtable_accessor_end()
{
    return R"cpp(}
                 ;
                 return &vtable;
                 }
    )cpp";
}

inline std::string
format_vtable_field_destroy(std::string_view slot_name, std::string_view cc_class_name)
{
    return std::format(
        R"(
            .{0} = [](void* plugin_context) noexcept {{
                delete {1}::create(plugin_context);
            }},
        )",
        slot_name,
        cc_class_name
    );
}

inline std::string
format_vtable_field_get_name(std::string_view slot_name, std::string_view cc_class_name)
{
    return std::format(
        R"(
            .{0} = [](void* plugin_context, TF_String* out) noexcept {{
                auto* self = {1}::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            }},
        )",
        slot_name,
        cc_class_name
    );
}

inline std::string format_vtable_field_generic_start(std::string_view slot_name)
{
    return std::format(".{} = [](", slot_name);
}

inline std::string
format_vtable_field_generic_middle(std::string_view cc_class_name, std::string_view slot_name)
{
    return std::format(
        R"(
            ) noexcept {{
                auto* self = {}::create(plugin_context);
                auto res = self->{}()",
        cc_class_name,
        slot_name
    );
}

inline std::string format_vtable_field_generic_end(std::string_view status_name)
{
    return std::format(
        R"(
                );
                if (!res) {{
                    res.error().to_c({});
                }}
            }},
        )",
        status_name
    );
}

inline std::string
format_method_body_start(std::string_view method_name, std::string_view namespace_name)
{
    return std::format(
        R"(
            ) noexcept
            {{
                {}::Status status;
                m_ops->{}(get_handle(), )",
        namespace_name,
        method_name
    );
}

constexpr std::string_view format_method_body_end()
{
    return R"(
                status.get_handle());
                
                if (!status.ok()) {
                    return std::unexpected{status};
                }
                return {};
            }
    )";
}
} // namespace cc_abi_gen::helper

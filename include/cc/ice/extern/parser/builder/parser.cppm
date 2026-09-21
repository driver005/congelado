// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parser.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parser.h"

export module cc_ice_extern_parser_builder:parser;

import std;

export namespace ice::builder {

class TF_ParserOps
{
public:
    static TF_ParserOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ParserOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ParserOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ParserOps*>(handle->plugin_data);
    }

    virtual ~TF_ParserOps() = default;

    static TF_ParserOps* get_generic_vtable()
    {
        static TF_ParserOps vtable = {
            .struct_size = TF_PARSER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ParserOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_ParserOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder

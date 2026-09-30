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
    TF_ParserOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ParserOps(const TF_ParserOps&) = delete;
    TF_ParserOps& operator=(const TF_ParserOps&) = delete;

    static TF_ParserOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ParserOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ParserOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ParserOps*>(handle->plugin_data);
    }

    virtual ~TF_ParserOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ParserOps{
            .struct_size = TF_PARSER_STRUCT_SIZE,
            .destroy =
                [](TF_Parser* parser) noexcept
            {
                TF_ParserOps::from_handle(parser).destroy();
            },
            .get_name =
                [](TF_Parser* parser, TF_String* out_name) noexcept
            {
                TF_ParserOps::from_handle(parser).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_ParserOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Parser& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ParserOps m_vtable;
    TF_Parser m_handle;
};

} // namespace ice::builder

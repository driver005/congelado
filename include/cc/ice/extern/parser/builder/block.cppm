// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"

export module cc_ice_extern_parser_builder:block;

import std;

export namespace ice::builder {

class TFParserBlockOps
{
public:
    TFParserBlockOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserBlockOps(const TFParserBlockOps&) = delete;
    TFParserBlockOps& operator=(const TFParserBlockOps&) = delete;

    static TFParserBlockOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserBlockOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserBlockOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserBlockOps*>(handle->plugin_data);
    }

    virtual ~TFParserBlockOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_node_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_node(int index, const ice::sonic::TFParserNodeOps& out_node) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserBlockOps{
            .struct_size = TF_ARSERBLOCK_STRUCT_SIZE,
            .get_name =
                [](TFParserBlock* block, TF_String* out_name, TF_Status* out_status) noexcept
            {
                auto res = TFParserBlockOps::from_handle(block).get_name(
                    ice::sonic::String::wrap(out_name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node_count =
                [](TFParserBlock* block, int* out_count, TF_Status* out_status) noexcept
            {
                auto res = TFParserBlockOps::from_handle(block).get_node_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node =
                [](TFParserBlock* block,
                   int index,
                   TFParserNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto res = TFParserBlockOps::from_handle(block).get_node(
                    index,
                    ice::sonic::TFParserNodeOps::wrap(out_node)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserBlockOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFParserBlock& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserBlockOps m_vtable;
    TFParserBlock m_handle;
};

} // namespace ice::builder

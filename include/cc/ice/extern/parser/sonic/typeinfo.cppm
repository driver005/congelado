// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/typeinfo.h"

export module cc_ice_extern_parser_sonic:typeinfo;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFParserTypeInfoOps : public ice::sonic::Runtime<TFParserTypeInfoOps, TFParserTypeInfoOps>
{
public:
    explicit TFParserTypeInfoOps(TFParserTypeInfoOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "parser";

    [[nodiscard]] std::expected<void, ice::Status> get_dtype(int* out_dtype) noexcept
    {
        ice::Status status;
        m_ops->get_dtype(get_handle(), out_dtype, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_shape(int64_t** out_dims, int* out_num_dims) noexcept
    {
        ice::Status status;
        m_ops->get_shape(get_handle(), out_dims, out_num_dims, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic

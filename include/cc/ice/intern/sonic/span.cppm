// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/span.h"

export module cc_ice_intern_sonic:span;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_SpanOps : public ice::sonic::Runtime<TF_SpanOps, TF_SpanOps>
{
public:
    explicit TF_SpanOps(TF_SpanOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get(size_t index, const void** out_value) noexcept
    {
        ice::sonic::Status status;
        m_ops->get(get_handle(), index, out_value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void size(size_t* out_size) noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void data(void** out_data) noexcept
    {
        m_ops->data(get_handle(), out_data);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    subspan(size_t offset, size_t count, const ice::sonic::TF_SpanOps& out_span) noexcept
    {
        ice::sonic::Status status;
        m_ops->subspan(get_handle(), offset, count, out_span.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic

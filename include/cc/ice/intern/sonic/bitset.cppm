// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/bitset.h"

export module cc_ice_intern_sonic:bitset;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_BitSetOps : public ice::sonic::Runtime<TF_BitSetOps, TF_BitSetOps>
{
public:
    explicit TF_BitSetOps(TF_BitSetOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void set(size_t index) noexcept
    {
        m_ops->set(get_handle(), index);
    }

    void clear(size_t index) noexcept
    {
        m_ops->clear(get_handle(), index);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    test(size_t index, int* out_result) noexcept
    {
        ice::sonic::Status status;
        m_ops->test(get_handle(), index, out_result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> flip(size_t index) noexcept
    {
        ice::sonic::Status status;
        m_ops->flip(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void count(size_t* out_count) noexcept
    {
        m_ops->count(get_handle(), out_count);
    }

    void size(size_t* out_size) noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic

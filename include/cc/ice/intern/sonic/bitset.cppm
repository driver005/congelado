// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/bitset.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_sonic:bitset;

import std;
import :registration;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class BitSet : public ice::sonic::Runtime<::TF_BitSetOps, ::TF_BitSet>
{
public:
    BitSet(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    BitSet(
        const ice::sonic::Registration& registry,
        const ::TF_BitSet& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit BitSet(const ::TF_BitSetOps* ops) noexcept :
        Runtime(ops)
    {
    }

    BitSet(const ::TF_BitSetOps* ops, const ::TF_BitSet* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void set(size_t index) const noexcept
    {
        m_ops->set(get_handle(), index);
    }

    void clear(size_t index) const noexcept
    {
        m_ops->clear(get_handle(), index);
    }

    void test(size_t index, int& out_result, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->test(get_handle(), index, &out_result, out_status.get_handle());
    }

    void flip(size_t index, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->flip(get_handle(), index, out_status.get_handle());
    }

    void count(std::size_t& out_count) const noexcept
    {
        m_ops->count(get_handle(), &out_count);
    }

    void size(std::size_t& out_size) const noexcept
    {
        m_ops->size(get_handle(), &out_size);
    }
};

} // namespace ice::sonic

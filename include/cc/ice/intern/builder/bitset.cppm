// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/bitset.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:bitset;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_BitSetOps
{
public:
    explicit TF_BitSetOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_BitSetOps(const TF_BitSetOps&) = delete;
    TF_BitSetOps& operator=(const TF_BitSetOps&) = delete;

    static TF_BitSetOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_BitSetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_BitSetOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_BitSetOps*>(handle->plugin_data);
    }

    virtual ~TF_BitSetOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void set(size_t index) noexcept = 0;
    virtual void clear(size_t index) noexcept = 0;
    virtual void
    test(size_t index, int* out_result, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void flip(size_t index, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void count(size_t* out_count) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_BitSet*)) noexcept
    {
        m_vtable = ::TF_BitSetOps{
            .struct_size = TF_OFFSET_OF_END(::TF_BitSetOps, size),

            .create = create,
            .destroy =
                [](TF_BitSet* handle) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(handle);
                self.destroy();
            },
            .set =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.set(index);
            },
            .clear =
                [](TF_BitSet* bitset, size_t index) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.clear(index);
            },
            .test =
                [](const TF_BitSet* bitset,
                   size_t index,
                   int* out_result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.test(
                    index,
                    out_result,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .flip =
                [](TF_BitSet* bitset, size_t index, TF_Status* out_status) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.flip(index, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .count =
                [](const TF_BitSet* bitset, size_t* out_count) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.count(out_count);
            },
            .size =
                [](const TF_BitSet* bitset, size_t* out_size) noexcept
            {
                auto& self = TF_BitSetOps::from_handle(bitset);
                self.size(out_size);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_BitSetOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_BitSet& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_BitSetOps*>(&m_vtable));
    }

private:
    ::TF_BitSetOps m_vtable;
    ::TF_BitSet m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

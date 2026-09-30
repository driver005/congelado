// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/set.h"

export module cc_ice_intern_builder:set;

import std;

export namespace ice::builder {

class TF_SetOps
{
public:
    TF_SetOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_SetOps(const TF_SetOps&) = delete;
    TF_SetOps& operator=(const TF_SetOps&) = delete;

    static TF_SetOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SetOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SetOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SetOps*>(handle->plugin_data);
    }

    virtual ~TF_SetOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    insert(const void* key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    find(const void* key, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    erase(const void* key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    contains(const void* key, int* out_found) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void for_each(TF_SetVisitor visitor, void* capture) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_SetOps{
            .struct_size = TF_SET_STRUCT_SIZE,
            .insert =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto res = TF_SetOps::from_handle(set).insert(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find =
                [](const TF_Set* set,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SetOps::from_handle(set).find(key, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Set* set, const void* key, TF_Status* out_status) noexcept
            {
                auto res = TF_SetOps::from_handle(set).erase(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .contains =
                [](const TF_Set* set,
                   const void* key,
                   int* out_found,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SetOps::from_handle(set).contains(key, out_found);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Set* set, size_t* out_size) noexcept
            {
                TF_SetOps::from_handle(set).size(out_size);
            },
            .for_each =
                [](const TF_Set* set, TF_SetVisitor visitor, void* capture) noexcept
            {
                TF_SetOps::from_handle(set).for_each(visitor, capture);
            },
            .destroy =
                [](TF_Set* set) noexcept
            {
                TF_SetOps::from_handle(set).destroy();
            },

        };
    }

    const ::TF_SetOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Set& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_SetOps m_vtable;
    TF_Set m_handle;
};

} // namespace ice::builder

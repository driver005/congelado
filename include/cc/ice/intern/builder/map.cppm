// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/map.h"

export module cc_ice_intern_builder:map;

import std;

export namespace ice::builder {

class TF_MapOps
{
public:
    TF_MapOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_MapOps(const TF_MapOps&) = delete;
    TF_MapOps& operator=(const TF_MapOps&) = delete;

    static TF_MapOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_MapOps*>(ctx);
    }

    template<typename HandleT>
    static TF_MapOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_MapOps*>(handle->plugin_data);
    }

    virtual ~TF_MapOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    insert(const void* key, const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    find(const void* key, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> erase(const void* key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    contains(const void* key, int* out_found) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    for_each(TF_MapVisitor visitor, void* capture) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_MapOps{
            .struct_size = TF_MAP_STRUCT_SIZE,
            .insert =
                [](TF_Map* map, const void* key, const void* value, TF_Status* out_status) noexcept
            {
                auto res = TF_MapOps::from_handle(map).insert(key, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find =
                [](const TF_Map* map,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_MapOps::from_handle(map).find(key, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_Map* map, const void* key, TF_Status* out_status) noexcept
            {
                auto res = TF_MapOps::from_handle(map).erase(key);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .contains =
                [](TF_Map* map, const void* key, int* out_found, TF_Status* out_status) noexcept
            {
                auto res = TF_MapOps::from_handle(map).contains(key, out_found);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Map* map, size_t* out_size) noexcept
            {
                auto res = TF_MapOps::from_handle(map).size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Map* map, TF_MapVisitor visitor, void* capture) noexcept
            {
                auto res = TF_MapOps::from_handle(map).for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_MapOps>{&TF_MapOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_MapOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Map& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_MapOps m_vtable;
    TF_Map m_handle;
};

} // namespace ice::builder

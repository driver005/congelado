// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/map.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:map;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_MapOps
{
public:
    explicit TF_MapOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void
    insert(const void* key, const void* value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void find(
        const void* key,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase(const void* key, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    contains(const void* key, int* out_found, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void for_each(TF_MapVisitor visitor, void* capture) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Map*)) noexcept
    {
        m_vtable = ::TF_MapOps{
            .struct_size = TF_OFFSET_OF_END(::TF_MapOps, for_each),

            .create = create,
            .destroy =
                [](TF_Map* handle) noexcept
            {
                auto& self = TF_MapOps::from_handle(handle);
                self.destroy();
            },
            .insert =
                [](TF_Map* map, const void* key, const void* value, TF_Status* out_status) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.insert(
                    key,
                    value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .find =
                [](const TF_Map* map,
                   const void* key,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.find(
                    key,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase =
                [](TF_Map* map, const void* key, TF_Status* out_status) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.erase(key, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .contains =
                [](TF_Map* map, const void* key, int* out_found, TF_Status* out_status) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.contains(
                    key,
                    out_found,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Map* map, size_t* out_size) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.size(out_size);
            },
            .for_each =
                [](const TF_Map* map, TF_MapVisitor visitor, void* capture) noexcept
            {
                auto& self = TF_MapOps::from_handle(map);
                self.for_each(visitor, capture);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_MapOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Map& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_MapOps*>(&m_vtable));
    }

private:
    ::TF_MapOps m_vtable;
    ::TF_Map m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

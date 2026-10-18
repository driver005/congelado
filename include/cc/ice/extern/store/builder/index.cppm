// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/index.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/index.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_store_builder:index;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class StoreIndex
{
public:
    explicit StoreIndex(
        const ::TF_MapOps* Map_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* Vector_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Map_ops = Map_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_Vector_ops = Vector_ops;
    }

    StoreIndex(const StoreIndex&) = delete;
    StoreIndex& operator=(const StoreIndex&) = delete;

    static StoreIndex& from_handle(void* ctx) noexcept
    {
        return *static_cast<StoreIndex*>(ctx);
    }

    template<typename HandleT>
    static StoreIndex& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<StoreIndex*>(handle->plugin_data);
    }

    virtual ~StoreIndex() = default;
    virtual void destroy() noexcept = 0;
    virtual void create_index(
        const ice::sonic::String& name,
        const ice::sonic::Map& field_config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    list(const ice::sonic::Vector& out_names, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreIndex*)) noexcept
    {
        m_vtable = ::TFStoreIndexOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreIndexOps, list),

            .create = create,
            .destroy =
                [](::TFStoreIndex* handle) noexcept
            {
                auto& self = StoreIndex::from_handle(handle);
                self.destroy();
            },
            .create_index =
                [](::TFStoreIndex* index,
                   const ::TF_String* name,
                   const ::TF_Map* field_config,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = StoreIndex::from_handle(index);
                self.create_index(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, field_config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .drop =
                [](::TFStoreIndex* index, const ::TF_String* name, ::TF_Status* out_status) noexcept
            {
                auto& self = StoreIndex::from_handle(index);
                self.drop(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list =
                [](::TFStoreIndex* index, ::TF_Vector* out_names, ::TF_Status* out_status) noexcept
            {
                auto& self = StoreIndex::from_handle(index);
                self.list(
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_names),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Map wrap(std::type_identity<ice::sonic::Map>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::Map{m_Map_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    ice::sonic::Vector
    wrap(std::type_identity<ice::sonic::Vector>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::Vector{m_Vector_ops, handle};
    }

    const ::TFStoreIndexOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreIndex& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreIndexOps*>(&m_vtable));
    }

private:
    ::TFStoreIndexOps m_vtable;
    ::TFStoreIndex m_handle;

    const ::TF_MapOps* m_Map_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_Vector_ops{nullptr};
};

} // namespace ice::builder

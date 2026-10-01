// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/item.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_grappler_builder:item;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerItemOps
{
public:
    explicit TFGrapplerItemOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TFGrapplerItemOps(const TFGrapplerItemOps&) = delete;
    TFGrapplerItemOps& operator=(const TFGrapplerItemOps&) = delete;

    static TFGrapplerItemOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerItemOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerItemOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerItemOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerItemOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_nodes_to_preserve_size(
        int* out_num_values,
        size_t* out_storage_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_nodes_to_preserve_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_fetch_nodes_size(
        int* out_num_values,
        size_t* out_storage_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_fetch_nodes_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerItem*)) noexcept
    {
        m_vtable = ::TFGrapplerItemOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerItemOps, get_fetch_nodes_list),

            .create = create,
            .destroy =
                [](TFGrapplerItem* handle) noexcept
            {
                auto& self = TFGrapplerItemOps::from_handle(handle);
                self.destroy();
            },
            .get_nodes_to_preserve_size =
                [](TFGrapplerItem* item,
                   int* out_num_values,
                   size_t* out_storage_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerItemOps::from_handle(item);
                self.get_nodes_to_preserve_size(
                    out_num_values,
                    out_storage_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_nodes_to_preserve_list =
                [](TFGrapplerItem* item,
                   char** out_values,
                   size_t* out_lengths,
                   int num_values,
                   void* storage,
                   size_t storage_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerItemOps::from_handle(item);
                self.get_nodes_to_preserve_list(
                    out_values,
                    out_lengths,
                    num_values,
                    storage,
                    storage_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_fetch_nodes_size =
                [](TFGrapplerItem* item,
                   int* out_num_values,
                   size_t* out_storage_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerItemOps::from_handle(item);
                self.get_fetch_nodes_size(
                    out_num_values,
                    out_storage_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_fetch_nodes_list =
                [](TFGrapplerItem* item,
                   char** out_values,
                   size_t* out_lengths,
                   int num_values,
                   void* storage,
                   size_t storage_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerItemOps::from_handle(item);
                self.get_fetch_nodes_list(
                    out_values,
                    out_lengths,
                    num_values,
                    storage,
                    storage_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TFGrapplerItemOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerItem& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFGrapplerItemOps*>(&m_vtable));
    }

private:
    ::TFGrapplerItemOps m_vtable;
    ::TFGrapplerItem m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

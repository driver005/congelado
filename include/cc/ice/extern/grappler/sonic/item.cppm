// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/item.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_grappler_sonic:item;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGrapplerItemOps : public ice::sonic::Runtime<::TFGrapplerItemOps, ::TFGrapplerItem>
{
public:
    TFGrapplerItemOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGrapplerItemOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGrapplerItem* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGrapplerItemOps(const ::TFGrapplerItemOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGrapplerItemOps(const ::TFGrapplerItemOps* ops, ::TFGrapplerItem* handle) noexcept :
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

    void get_nodes_to_preserve_size(
        int* out_num_values,
        size_t* out_storage_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_nodes_to_preserve_size(
            get_handle(),
            out_num_values,
            out_storage_size,
            out_status.get_handle()
        );
    }

    void get_nodes_to_preserve_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_nodes_to_preserve_list(
            get_handle(),
            out_values,
            out_lengths,
            num_values,
            storage,
            storage_size,
            out_status.get_handle()
        );
    }

    void get_fetch_nodes_size(
        int* out_num_values,
        size_t* out_storage_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_fetch_nodes_size(
            get_handle(),
            out_num_values,
            out_storage_size,
            out_status.get_handle()
        );
    }

    void get_fetch_nodes_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_fetch_nodes_list(
            get_handle(),
            out_values,
            out_lengths,
            num_values,
            storage,
            storage_size,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic

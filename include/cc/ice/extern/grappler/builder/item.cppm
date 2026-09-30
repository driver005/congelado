// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/item.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"

export module cc_ice_extern_grappler_builder:item;

import std;

export namespace ice::builder {

class TFGrapplerItemOps
{
public:
    TFGrapplerItemOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_nodes_to_preserve_size(int* out_num_values, size_t* out_storage_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_nodes_to_preserve_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_fetch_nodes_size(int* out_num_values, size_t* out_storage_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_fetch_nodes_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerItemOps{
            .struct_size = TF_RAPPLERITEM_STRUCT_SIZE,
            .get_nodes_to_preserve_size =
                [](TFGrapplerItem* item,
                   int* out_num_values,
                   size_t* out_storage_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerItemOps::from_handle(item).get_nodes_to_preserve_size(
                    out_num_values,
                    out_storage_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TFGrapplerItemOps::from_handle(item).get_nodes_to_preserve_list(
                    out_values,
                    out_lengths,
                    num_values,
                    storage,
                    storage_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_fetch_nodes_size =
                [](TFGrapplerItem* item,
                   int* out_num_values,
                   size_t* out_storage_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerItemOps::from_handle(item).get_fetch_nodes_size(
                    out_num_values,
                    out_storage_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TFGrapplerItemOps::from_handle(item).get_fetch_nodes_list(
                    out_values,
                    out_lengths,
                    num_values,
                    storage,
                    storage_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerItemOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerItem& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerItemOps m_vtable;
    TFGrapplerItem m_handle;
};

} // namespace ice::builder

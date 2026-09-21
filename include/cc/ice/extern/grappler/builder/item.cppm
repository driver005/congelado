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
    static TFGrapplerItemOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerItemOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerItemOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerItemOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerItemOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_nodes_to_preserve_size(int* out_num_values, size_t* out_storage_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_nodes_to_preserve_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_fetch_nodes_size(int* out_num_values, size_t* out_storage_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_fetch_nodes_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;

    static TFGrapplerItemOps* get_generic_vtable()
    {
        static TFGrapplerItemOps vtable = {
            .struct_size = TF_RAPPLERITEM_STRUCT_SIZE,
            .get_nodes_to_preserve_size =
                [](TFGrapplerItem* item,
                   int* out_num_values,
                   size_t* out_storage_size,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerItemOps::create(item);
                auto res = self->get_nodes_to_preserve_size(out_num_values, out_storage_size);
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
                auto* self = TFGrapplerItemOps::create(item);
                auto res = self->get_nodes_to_preserve_list(
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
                auto* self = TFGrapplerItemOps::create(item);
                auto res = self->get_fetch_nodes_size(out_num_values, out_storage_size);
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
                auto* self = TFGrapplerItemOps::create(item);
                auto res = self->get_fetch_nodes_list(
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

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder

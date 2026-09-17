// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/list/list.h"

export module cc_abi_builder_list;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ListOps
{
public:
    static TF_ListOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ListOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ListOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ListOps*>(handle->plugin_data);
    }

    virtual ~TF_ListOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    push_front(const void* value, TFListNode* out_node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    push_back(const void* value, TFListNode* out_node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(TFListNode* node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_ListVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_ListOps* get_generic_vtable()
    {
        static TF_ListOps vtable = {
            .struct_size = TF_LIST_STRUCT_SIZE,
            .set_element_size =
                [](TF_List* list, size_t element_size) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_List* list,
                   const void* value,
                   TFListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->push_front(value, out_node);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .push_back =
                [](TF_List* list,
                   const void* value,
                   TFListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->push_back(value, out_node);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_List* list, TFListNode* node) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->erase(node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_List* list, TF_ListVisitor visitor, void* capture) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_List* list, size_t* out_size) noexcept
            {
                auto* self = TF_ListOps::create(list);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ListOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

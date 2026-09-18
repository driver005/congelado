// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/forward_list/forward_list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/forward_list/forward_list.h"

export module cc_ice_builder_forward_list:forward_list;

import std;

export namespace ice::builder {

class TF_ForwardListOps
{
public:
    static TF_ForwardListOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ForwardListOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ForwardListOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ForwardListOps*>(handle->plugin_data);
    }

    virtual ~TF_ForwardListOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    push_front(const void* value, TFForwardListNode* out_node) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    erase_after(TFForwardListNode* node) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    for_each(TF_ForwardListVisitor visitor, void* capture) noexcept = 0;

    static TF_ForwardListOps* get_generic_vtable()
    {
        static TF_ForwardListOps vtable = {
            .struct_size = TF_FORWARDLIST_STRUCT_SIZE,
            .set_element_size =
                [](TF_ForwardList* list, size_t element_size) noexcept
            {
                auto* self = TF_ForwardListOps::create(list);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_ForwardList* list,
                   const void* value,
                   TFForwardListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ForwardListOps::create(list);
                auto res = self->push_front(value, out_node);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase_after =
                [](TF_ForwardList* list, TFForwardListNode* node) noexcept
            {
                auto* self = TF_ForwardListOps::create(list);
                auto res = self->erase_after(node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_ForwardList* list,
                   TF_ForwardListVisitor visitor,
                   void* capture) noexcept
            {
                auto* self = TF_ForwardListOps::create(list);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ForwardListOps::create(plugin_context);
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

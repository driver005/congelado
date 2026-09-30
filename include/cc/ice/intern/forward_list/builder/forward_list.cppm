// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/forward_list/forward_list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/forward_list/forward_list.h"

export module cc_ice_intern_forward_list_builder:forward_list;

import std;

export namespace ice::builder {

class TF_ForwardListOps
{
public:
    TF_ForwardListOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ForwardListOps(const TF_ForwardListOps&) = delete;
    TF_ForwardListOps& operator=(const TF_ForwardListOps&) = delete;

    static TF_ForwardListOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ForwardListOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ForwardListOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ForwardListOps*>(handle->plugin_data);
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

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ForwardListOps{
            .struct_size = TF_FORWARDLIST_STRUCT_SIZE,
            .set_element_size =
                [](TF_ForwardList* list, size_t element_size) noexcept
            {
                auto res = TF_ForwardListOps::from_handle(list).set_element_size(element_size);
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
                auto res = TF_ForwardListOps::from_handle(list).push_front(value, out_node);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase_after =
                [](TF_ForwardList* list, TFForwardListNode* node) noexcept
            {
                auto res = TF_ForwardListOps::from_handle(list).erase_after(node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_ForwardList* list,
                   TF_ForwardListVisitor visitor,
                   void* capture) noexcept
            {
                auto res = TF_ForwardListOps::from_handle(list).for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_ForwardListOps>{&TF_ForwardListOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_ForwardListOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_ForwardList& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ForwardListOps m_vtable;
    TF_ForwardList m_handle;
};

} // namespace ice::builder

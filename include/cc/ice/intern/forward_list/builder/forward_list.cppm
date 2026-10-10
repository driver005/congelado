// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/forward_list/forward_list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_forward_list_builder:forward_list;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ForwardListOps
{
public:
    explicit TF_ForwardListOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void push_front(
        const void* value,
        TFForwardListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase_after(TFForwardListNode* node) noexcept = 0;
    virtual void for_each(TF_ForwardListVisitor visitor, void* capture) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_ForwardList*)) noexcept
    {
        m_vtable = ::TF_ForwardListOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ForwardListOps, for_each),

            .create = create,
            .destroy =
                [](TF_ForwardList* handle) noexcept
            {
                auto& self = TF_ForwardListOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_ForwardList* list, size_t element_size) noexcept
            {
                auto& self = TF_ForwardListOps::from_handle(list);
                self.set_element_size(element_size);
            },
            .push_front =
                [](TF_ForwardList* list,
                   const void* value,
                   TFForwardListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ForwardListOps::from_handle(list);
                self.push_front(
                    value,
                    out_node,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase_after =
                [](TF_ForwardList* list, TFForwardListNode* node) noexcept
            {
                auto& self = TF_ForwardListOps::from_handle(list);
                self.erase_after(node);
            },
            .for_each =
                [](const TF_ForwardList* list,
                   TF_ForwardListVisitor visitor,
                   void* capture) noexcept
            {
                auto& self = TF_ForwardListOps::from_handle(list);
                self.for_each(visitor, capture);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_ForwardListOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_ForwardList& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_ForwardListOps*>(&m_vtable)
        );
    }

private:
    ::TF_ForwardListOps m_vtable;
    ::TF_ForwardList m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

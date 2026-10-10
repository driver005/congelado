// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/list/list.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_list_builder:list;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ListOps
{
public:
    explicit TF_ListOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_ListOps(const TF_ListOps&) = delete;
    TF_ListOps& operator=(const TF_ListOps&) = delete;

    static TF_ListOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ListOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ListOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ListOps*>(handle->plugin_data);
    }

    virtual ~TF_ListOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void push_front(
        const void* value,
        TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void push_back(
        const void* value,
        TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void erase(TFListNode* node) noexcept = 0;
    virtual void for_each(TF_ListVisitor visitor, void* capture) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_List*)) noexcept
    {
        m_vtable = ::TF_ListOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ListOps, size),

            .create = create,
            .destroy =
                [](TF_List* handle) noexcept
            {
                auto& self = TF_ListOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_List* list, size_t element_size) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.set_element_size(element_size);
            },
            .push_front =
                [](TF_List* list,
                   const void* value,
                   TFListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.push_front(
                    value,
                    out_node,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .push_back =
                [](TF_List* list,
                   const void* value,
                   TFListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.push_back(
                    value,
                    out_node,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .erase =
                [](TF_List* list, TFListNode* node) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.erase(node);
            },
            .for_each =
                [](const TF_List* list, TF_ListVisitor visitor, void* capture) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.for_each(visitor, capture);
            },
            .size =
                [](const TF_List* list, size_t* out_size) noexcept
            {
                auto& self = TF_ListOps::from_handle(list);
                self.size(out_size);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_ListOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_List& get_handle() const noexcept
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
            const_cast<::TF_ListOps*>(&m_vtable)
        );
    }

private:
    ::TF_ListOps m_vtable;
    ::TF_List m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

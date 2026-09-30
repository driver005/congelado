// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/list/list.h"

export module cc_ice_intern_list_builder:list;

import std;

export namespace ice::builder {

class TF_ListOps
{
public:
    TF_ListOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    virtual void set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    push_front(const void* value, TFListNode* out_node) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    push_back(const void* value, TFListNode* out_node) noexcept = 0;
    virtual void erase(TFListNode* node) noexcept = 0;
    virtual void for_each(TF_ListVisitor visitor, void* capture) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ListOps{
            .struct_size = TF_LIST_STRUCT_SIZE,
            .set_element_size =
                [](TF_List* list, size_t element_size) noexcept
            {
                TF_ListOps::from_handle(list).set_element_size(element_size);
            },
            .push_front =
                [](TF_List* list,
                   const void* value,
                   TFListNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ListOps::from_handle(list).push_front(value, out_node);
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
                auto res = TF_ListOps::from_handle(list).push_back(value, out_node);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .erase =
                [](TF_List* list, TFListNode* node) noexcept
            {
                TF_ListOps::from_handle(list).erase(node);
            },
            .for_each =
                [](const TF_List* list, TF_ListVisitor visitor, void* capture) noexcept
            {
                TF_ListOps::from_handle(list).for_each(visitor, capture);
            },
            .size =
                [](const TF_List* list, size_t* out_size) noexcept
            {
                TF_ListOps::from_handle(list).size(out_size);
            },
            .destroy =
                [](TF_List* list) noexcept
            {
                TF_ListOps::from_handle(list).destroy();
            },

        };
    }

    const ::TF_ListOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_List& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ListOps m_vtable;
    TF_List m_handle;
};

} // namespace ice::builder

// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/forwardlist/forwardlist.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/forwardlist/forwardlist.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_forwardlist;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Forwardlist
{
public:
    static Forwardlist* create(void* ctx) noexcept
    {
        return static_cast<Forwardlist*>(ctx);
    }

    template<typename HandleT>
    static Forwardlist* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Forwardlist*>(handle);
    }

    virtual ~Forwardlist() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_forward_list(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_front(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    erase_after(TF_ForwardList_Node* node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_ForwardListVisitor visitor, void* capture) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_ForwardList* get_generic_vtable()
    {
        static TF_ForwardList vtable = {
            .struct_size = TF_FORWARDLIST_STRUCT_SIZE,
            .new_forward_list =
                [](void* plugin_context, size_t element_size) noexcept
            {
                auto* self = Forwardlist::create(plugin_context);
                auto res = self->new_forward_list(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_ForwardList_Handle* list, const void* value) noexcept
            {
                auto* self = Forwardlist::create(list);
                auto res = self->push_front(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .erase_after =
                [](TF_ForwardList_Handle* list, TF_ForwardList_Node* node) noexcept
            {
                auto* self = Forwardlist::create(list);
                auto res = self->erase_after(node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_ForwardList_Handle* list,
                   TF_ForwardListVisitor visitor,
                   void* capture) noexcept
            {
                auto* self = Forwardlist::create(list);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Forwardlist::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/list/list.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_list;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class List
{
public:
    static List* create(void* ctx) noexcept
    {
        return static_cast<List*>(ctx);
    }

    template<typename HandleT>
    static List* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<List*>(handle);
    }

    virtual ~List() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_list(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_front(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(TF_List_Node* node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_ListVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_List* get_generic_vtable()
    {
        static TF_List vtable = {
            .struct_size = TF_LIST_STRUCT_SIZE,
            .new_list =
                [](void* plugin_context, size_t element_size) noexcept
            {
                auto* self = List::create(plugin_context);
                auto res = self->new_list(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_List_Handle* list, const void* value) noexcept
            {
                auto* self = List::create(list);
                auto res = self->push_front(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_List_Handle* list, const void* value) noexcept
            {
                auto* self = List::create(list);
                auto res = self->push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .erase =
                [](TF_List_Handle* list, TF_List_Node* node) noexcept
            {
                auto* self = List::create(list);
                auto res = self->erase(node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_List_Handle* list, TF_ListVisitor visitor, void* capture) noexcept
            {
                auto* self = List::create(list);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_List_Handle* list) noexcept
            {
                auto* self = List::create(list);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete List::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

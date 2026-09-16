// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/deque/deque.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/deque/deque.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_deque;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Deque
{
public:
    static Deque* create(void* ctx) noexcept
    {
        return static_cast<Deque*>(ctx);
    }

    template<typename HandleT>
    static Deque* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Deque*>(handle);
    }

    virtual ~Deque() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_deque(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_front(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> pop_front() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> pop_back() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Deque* get_generic_vtable()
    {
        static TF_Deque vtable = {
            .struct_size = TF_DEQUE_STRUCT_SIZE,
            .new_deque =
                [](void* plugin_context, size_t element_size) noexcept
            {
                auto* self = Deque::create(plugin_context);
                auto res = self->new_deque(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_Deque_Handle* deque, const void* value) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->push_front(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_Deque_Handle* deque, const void* value) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_front =
                [](TF_Deque_Handle* deque) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->pop_front();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_back =
                [](TF_Deque_Handle* deque) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->pop_back();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Deque_Handle* deque, size_t index) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->get(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Deque_Handle* deque) noexcept
            {
                auto* self = Deque::create(deque);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Deque::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

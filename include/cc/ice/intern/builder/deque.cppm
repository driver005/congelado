// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/deque.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/deque.h"

export module cc_ice_intern_builder:deque;

import std;

export namespace ice::builder {

class TF_DequeOps
{
public:
    TF_DequeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_DequeOps(const TF_DequeOps&) = delete;
    TF_DequeOps& operator=(const TF_DequeOps&) = delete;

    static TF_DequeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DequeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DequeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DequeOps*>(handle->plugin_data);
    }

    virtual ~TF_DequeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    push_front(const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    push_back(const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> pop_front() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> pop_back() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DequeOps{
            .struct_size = TF_DEQUE_STRUCT_SIZE,
            .set_element_size =
                [](TF_Deque* deque, size_t element_size) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).push_front(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_front =
                [](TF_Deque* deque) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).pop_front();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_back =
                [](TF_Deque* deque) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).pop_back();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Deque* deque,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Deque* deque, size_t* out_size) noexcept
            {
                auto res = TF_DequeOps::from_handle(deque).size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_DequeOps>{&TF_DequeOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_DequeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Deque& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DequeOps m_vtable;
    TF_Deque m_handle;
};

} // namespace ice::builder

// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/deque.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/deque.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:deque;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_DequeOps
{
public:
    explicit TF_DequeOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void push_front(const void* value) noexcept = 0;
    virtual void push_back(const void* value) noexcept = 0;
    virtual void pop_front() noexcept = 0;
    virtual void pop_back() noexcept = 0;
    virtual void
    get(size_t index, const void** out_value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Deque*)) noexcept
    {
        m_vtable = ::TF_DequeOps{
            .struct_size = TF_OFFSET_OF_END(::TF_DequeOps, size),

            .create = create,
            .destroy =
                [](TF_Deque* handle) noexcept
            {
                auto& self = TF_DequeOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_Deque* deque, size_t element_size) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.set_element_size(element_size);
            },
            .push_front =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.push_front(value);
            },
            .push_back =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.push_back(value);
            },
            .pop_front =
                [](TF_Deque* deque) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.pop_front();
            },
            .pop_back =
                [](TF_Deque* deque) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.pop_back();
            },
            .get =
                [](const TF_Deque* deque,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.get(
                    index,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Deque* deque, size_t* out_size) noexcept
            {
                auto& self = TF_DequeOps::from_handle(deque);
                self.size(out_size);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_DequeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Deque& get_handle() const noexcept
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
            const_cast<::TF_DequeOps*>(&m_vtable)
        );
    }

private:
    ::TF_DequeOps m_vtable;
    ::TF_Deque m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

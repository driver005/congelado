// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:stream;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_StreamOps
{
public:
    explicit TF_StreamOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_StreamOps(const TF_StreamOps&) = delete;
    TF_StreamOps& operator=(const TF_StreamOps&) = delete;

    static TF_StreamOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_StreamOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StreamOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_StreamOps*>(handle->plugin_data);
    }

    virtual ~TF_StreamOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_priority(int32_t* out_priority) noexcept = 0;
    virtual void get_device_index(int* out_device_index) noexcept = 0;
    virtual void query(_Bool* out_idle, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void synchronize(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_capture_status(
        TF_CaptureStatus* out_capture_status,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Stream*)) noexcept
    {
        m_vtable = ::TF_StreamOps{
            .struct_size = TF_OFFSET_OF_END(::TF_StreamOps, get_native_handle),

            .create = create,
            .destroy =
                [](TF_Stream* handle) noexcept
            {
                auto& self = TF_StreamOps::from_handle(handle);
                self.destroy();
            },
            .get_priority =
                [](TF_Stream* stream, int32_t* out_priority) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.get_priority(out_priority);
            },
            .get_device_index =
                [](TF_Stream* stream, int* out_device_index) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.get_device_index(out_device_index);
            },
            .query =
                [](TF_Stream* stream, _Bool* out_idle, TF_Status* out_status) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.query(
                    out_idle,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .synchronize =
                [](TF_Stream* stream, TF_Status* out_status) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.synchronize(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_capture_status =
                [](TF_Stream* stream,
                   TF_CaptureStatus* out_capture_status,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.get_capture_status(
                    out_capture_status,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](TF_Stream* stream, void** out_handle) noexcept
            {
                auto& self = TF_StreamOps::from_handle(stream);
                self.get_native_handle(out_handle);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_StreamOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Stream& get_handle() const noexcept
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
            const_cast<::TF_StreamOps*>(&m_vtable)
        );
    }

private:
    ::TF_StreamOps m_vtable;
    ::TF_Stream m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

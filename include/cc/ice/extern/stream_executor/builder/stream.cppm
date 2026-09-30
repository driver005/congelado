// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/stream.h"

export module cc_ice_extern_stream_executor_builder:stream;

import std;

export namespace ice::builder {

class TF_StreamOps
{
public:
    TF_StreamOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_priority(int32_t* out_priority) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_index(int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> query(_Bool* out_idle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> synchronize() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_capture_status(TF_CaptureStatus* out_capture_status) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StreamOps{
            .struct_size = TF_STREAM_STRUCT_SIZE,
            .get_priority =
                [](TF_Stream* stream, int32_t* out_priority) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).get_priority(out_priority);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_device_index =
                [](TF_Stream* stream, int* out_device_index) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).get_device_index(out_device_index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .query =
                [](TF_Stream* stream, _Bool* out_idle, TF_Status* out_status) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).query(out_idle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .synchronize =
                [](TF_Stream* stream, TF_Status* out_status) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).synchronize();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_capture_status =
                [](TF_Stream* stream,
                   TF_CaptureStatus* out_capture_status,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).get_capture_status(out_capture_status);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Stream* stream, void** out_handle) noexcept
            {
                auto res = TF_StreamOps::from_handle(stream).get_native_handle(out_handle);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_StreamOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Stream& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_StreamOps m_vtable;
    TF_Stream m_handle;
};

} // namespace ice::builder

module;

#include "include/c/extern/stream_executor/stream.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:stream;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclStream : public ice::builder::TF_StreamOps
{
public:
    explicit SyclStream(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_StreamOps{ops.getStatusOps()},
        m_status{ops}
    {
    }

    ~SyclStream() override = default;
    SyclStream(const SyclStream&) = delete;
    SyclStream& operator=(const SyclStream&) = delete;
    SyclStream(SyclStream&&) = delete;
    SyclStream& operator=(SyclStream&&) = delete;

    static void create(::TF_Stream* handle)
    {

        auto* stream = new SyclStream{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *stream);

    }

    static std::shared_ptr<sycl::queue> create_queue(
        const sycl::context& context,
        const sycl::device& device,
        int32_t priority,
        const std::shared_ptr<std::string>& async_error
    )
    {

        auto handler = SyclStatus::make_async_handler(async_error);
        if (priority < 0) {
            return std::make_shared<sycl::queue>(
                context,
                device,
                handler,
                sycl::property_list{
                    sycl::property::queue::in_order{},
                    sycl::ext::oneapi::property::queue::priority_high{}
                }
            );
        }
        if (priority > 0) {
            return std::make_shared<sycl::queue>(
                context,
                device,
                handler,
                sycl::property_list{
                    sycl::property::queue::in_order{},
                    sycl::ext::oneapi::property::queue::priority_low{}
                }
            );
        }
        return std::make_shared<sycl::queue>(
            context,
            device,
            handler,
            sycl::property_list{sycl::property::queue::in_order{}}
        );

    }

    void bind(
        std::shared_ptr<sycl::queue> queue,
        std::shared_ptr<std::string> async_error,
        int device_index,
        int32_t priority
    ) noexcept
    {

        m_queue = std::move(queue);
        m_async_error = std::move(async_error);
        m_device_index = device_index;
        m_priority = priority;

    }

    void release() noexcept
    {

        m_queue.reset();
        m_async_error.reset();

    }

    void destroy() noexcept override { delete this; }

    void get_priority(int32_t* out_priority) noexcept override { *out_priority = m_priority; }

    void get_device_index(int* out_device_index) noexcept override
    {

        *out_device_index = m_device_index;

    }

    void query(_Bool* out_idle, const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_queue) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "stream is not bound");
            return;
        }
        *out_idle = m_queue->ext_oneapi_empty();

    }

    void synchronize(const ice::sonic::Status& out_status) noexcept override
    {

        if (!m_queue) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "stream is not bound");
            return;
        }

        try {
            m_queue->wait_and_throw();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void get_capture_status(
        TF_CaptureStatus* out_capture_status,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        const bool recording =
            m_queue && m_queue->ext_oneapi_get_state() ==
                           sycl::ext::oneapi::experimental::queue_state::recording;
        *out_capture_status =
            recording || m_capturing ? TF_CAPTURE_STATUS_ACTIVE : TF_CAPTURE_STATUS_NONE;

    }

    void get_native_handle(void** out_handle) noexcept override { *out_handle = m_queue.get(); }

    void setCapturing(bool capturing) noexcept { m_capturing = capturing; }

    sycl::queue& getNativeQueue() const noexcept { return *m_queue; }

    const std::string& getAsyncError() const noexcept { return *m_async_error; }

    const std::shared_ptr<sycl::queue>& getSharedQueue() const noexcept { return m_queue; }

    const std::shared_ptr<std::string>& getSharedAsyncError() const noexcept
    {

        return m_async_error;

    }

    bool getCapturing() const noexcept { return m_capturing; }

private:
    SyclStatus m_status;
    std::shared_ptr<sycl::queue> m_queue;
    std::shared_ptr<std::string> m_async_error;
    int m_device_index{-1};
    int32_t m_priority{0};
    bool m_capturing{false};
};

} // namespace aten_xpu

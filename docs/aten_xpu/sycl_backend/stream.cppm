// SYCL reference plugin — one in-order sycl::queue per TF_Stream.
//
// Not built by Bazel (docs/ only). Owned by SyclExecutor's stream pool (see executor.cppm);
// never destroyed through destroy_stream_internal while borrowed from the pool, per the C
// header's own comment on TF_ExecutorOps::get_stream_from_pool.

module;

#include "include/c/extern/stream_executor/stream.h"

export module sycl_backend:stream;

import std;
import cc_ice_extern_stream_executor_builder;

export namespace sycl_backend {

class SyclStream : public ice::builder::Stream
{
public:
    SyclStream(sycl::queue&& queue, int device_index, int32_t priority) :
        m_queue{std::move(queue)},
        m_device_index{device_index},
        m_priority{priority}
    {
    }

    ~SyclStream() override = default;
    SyclStream(const SyclStream&) = delete;
    SyclStream& operator=(const SyclStream&) = delete;
    SyclStream(SyclStream&&) = delete;
    SyclStream& operator=(SyclStream&&) = delete;

    sycl::queue& get_native_queue() noexcept
    {
        return m_queue;
    }

    void get_priority(int32_t* out_priority) noexcept override
    {
        *out_priority = m_priority;
    }

    void get_device_index(int* out_device_index) noexcept override
    {
        *out_device_index = m_device_index;
    }

    // Non-blocking: ext_oneapi_empty(), not queue.wait() — the C header calls this a "query", and
    // a query that blocks the caller until the queue drains is not a query.
    [[nodiscard]] std::expected<void, ice::sonic::Status> query(bool* out_idle) noexcept override
    {
        *out_idle = m_queue.ext_oneapi_empty();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> synchronize() noexcept override
    {
        try {
            m_queue.wait_and_throw();
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_capture_status(TF_CaptureStatus* out_capture_status) noexcept override
    {
        *out_capture_status = m_capturing ? TF_CAPTURE_STATUS_ACTIVE : TF_CAPTURE_STATUS_NONE;
        return {};
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = &m_queue;
    }

    // Set by SyclDeviceGraph::capture_begin/capture_end (device_graph.cppm) while this stream is
    // the capture stream.
    void set_capturing(bool capturing) noexcept
    {
        m_capturing = capturing;
    }

private:
    sycl::queue m_queue;
    int m_device_index;
    int32_t m_priority;
    bool m_capturing{false};
};

} // namespace sycl_backend

module;

#include "include/c/extern/stream_executor/event.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:event;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclEvent : public ice::builder::TF_EventOps
{
public:
    explicit SyclEvent(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_EventOps{ops.getEventOps(), ops.getStatusOps()},
        m_status{ops}
    {
    }

    ~SyclEvent() override = default;
    SyclEvent(const SyclEvent&) = delete;
    SyclEvent& operator=(const SyclEvent&) = delete;
    SyclEvent(SyclEvent&&) = delete;
    SyclEvent& operator=(SyclEvent&&) = delete;

    static void create(::TF_Event* handle)
    {
        auto* event = new SyclEvent{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *event);
    }

    void bind(const TF_EventOptions& options, int device_index) noexcept
    {
        m_enable_timing = options.enable_timing;
        m_enable_ipc = options.enable_ipc;
        m_reusable = options.reusable || options.enable_ipc;
        m_device_index = device_index;
        m_event.reset();
    }

    void bind_from_ipc(
        const TF_IpcEventHandle& handle,
        const sycl::context& context,
        const sycl::device& device,
        int device_index
    )
    {
        namespace ipc = sycl::ext::oneapi::experimental::ipc;

        if (!device.has(sycl::aspect::ext_oneapi_ipc_event)) {
            throw std::runtime_error{"device does not support IPC events"};
        }

        const auto* bytes = reinterpret_cast<const std::byte*>(handle.data);
        const ipc::handle_data_t handle_data(bytes, bytes + handle.data_size);
        m_event = sycl::event{ipc::event::open(handle_data, context)};
        m_enable_ipc = true;
        m_reusable = true;
        m_imported = true;
        m_device_index = device_index;
    }

    void record(sycl::queue& queue)
    {
        m_event = queue.ext_oneapi_submit_barrier();
    }

    bool is_complete() const
    {
        return !m_event || m_event->get_info<sycl::info::event::command_execution_status>() ==
                               sycl::info::event_command_status::complete;
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void elapsed_time(
        const ice::sonic::TF_EventOps& end,
        float* out_milliseconds,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        auto& end_event = SyclHandle::resolve<SyclEvent>(end);
        if (!m_enable_timing || !end_event.m_enable_timing) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "both events need enable_timing");
            return;
        }
        if (!m_event || !end_event.m_event) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "both events must be recorded");
            return;
        }

        try {
            m_event->wait_and_throw();
            end_event.m_event->wait_and_throw();
            const auto start_nanoseconds =
                m_event->get_profiling_info<sycl::info::event_profiling::command_end>();
            const auto end_nanoseconds =
                end_event.m_event->get_profiling_info<sycl::info::event_profiling::command_end>();
            *out_milliseconds = static_cast<float>(end_nanoseconds - start_nanoseconds) / 1.0e6F;
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void export_ipc(
        TF_IpcEventHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        namespace ipc = sycl::ext::oneapi::experimental::ipc;

        if (!m_enable_ipc || m_imported) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "event was not created with ipc");
            return;
        }
        if (!m_event) {
            m_status.fail(out_status, TF_FAILED_PRECONDITION, "event must be recorded first");
            return;
        }

        try {
            const auto handle_data = ipc::event::get(*m_event).data();
            if (handle_data.size() > sizeof(out_handle->data)) {
                m_status.fail(out_status, TF_OUT_OF_RANGE, "ipc handle exceeds 64 bytes");
                return;
            }
            out_handle->struct_size = sizeof(TF_IpcEventHandle);
            std::memcpy(out_handle->data, handle_data.data(), handle_data.size());
            out_handle->data_size = handle_data.size();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = m_event ? &*m_event : nullptr;
    }

    const std::optional<sycl::event>& getNativeEvent() const noexcept
    {
        return m_event;
    }

    int getDeviceIndex() const noexcept
    {
        return m_device_index;
    }

private:
    SyclStatus m_status;
    std::optional<sycl::event> m_event;
    int m_device_index{-1};
    bool m_enable_timing{false};
    bool m_enable_ipc{false};
    bool m_reusable{false};
    bool m_imported{false};
};

} // namespace aten_xpu

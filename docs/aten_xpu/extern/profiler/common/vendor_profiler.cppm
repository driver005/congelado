module;

#include "docs/aten_xpu/extern/profiler/xspace_writer.h"
#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

#include <unistd.h>

export module aten_xpu_profiler_common:vendor_profiler;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_profiler_builder;
import :backend;
import :device_activity;
import :host_ops;
import :host_tensor;
import :kind;
import :status;

export namespace aten_xpu {

// A profiler that only reports what one vendor tracer saw on the device. It does not use SYCL.
class VendorProfiler : public ice::builder::Profiler
{
public:
    VendorProfiler(const ProfilerHostOps& ops, ProfileBackend& backend, std::string device_type) :
        ice::builder::Profiler{ops.getStatusOps(), ops.getStringOps()},
        m_ops{ops},
        m_backend{backend},
        m_device_type{std::move(device_type)},
        m_status{ops}
    {
    }

    ~VendorProfiler() override = default;
    VendorProfiler(const VendorProfiler&) = delete;
    VendorProfiler& operator=(const VendorProfiler&) = delete;
    VendorProfiler(VendorProfiler&&) = delete;
    VendorProfiler& operator=(VendorProfiler&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy(std::format("aten_xpu_{}", m_backend.getName()));
    }

    void get_device_type(const ice::sonic::String& out_device_type) noexcept override
    {
        out_device_type.copy(m_device_type);
    }

    void start(const ice::sonic::Status& out_status) noexcept override
    {
        if (!m_backend.start()) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_failed_precondition,
                std::format("{} tracer unavailable on this machine", m_backend.getName())
            );
        }
    }

    void stop(const ice::sonic::Status& out_status) noexcept override
    {
        static_cast<void>(out_status);
        m_backend.stop();
    }

    void collect_data_xspace(
        ::TF_Tensor** out_data,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            const std::string BYTES = build_xspace();

            auto* tensor = new HostTensor{m_ops};
            tensor->assign(std::as_bytes(std::span{BYTES}));
            auto* handle = new ::TF_Tensor{};
            handle->plugin_data = tensor->get_handle().plugin_data;
            *out_data = handle;
        } catch (const std::exception& error) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error.what());
        }
    }

private:
    static std::string_view kind_name(ProfileKind kind) noexcept
    {
        switch (kind) {
            case ProfileKind::COPY:
                return "copy";
            case ProfileKind::HOST:
                return "host";
            case ProfileKind::COMPUTE:
                break;
        }

        return "compute";
    }

    // One XPlane per device the tracer reported, one XLine per queue and kind.
    std::string build_xspace()
    {
        XSpaceWriter writer;
        std::array<char, 256> hostname{};
        if (gethostname(hostname.data(), hostname.size() - 1) == 0) {
            writer.add_hostname(hostname.data());
        }

        std::map<std::string, std::size_t> planes;
        std::map<std::tuple<std::string, std::uint64_t, ProfileKind>, std::size_t> lines;
        for (const ProfileDeviceActivity& activity: m_backend.drain()) {
            auto plane = planes.find(activity.getDevice());
            if (plane == planes.end()) {
                plane = planes
                            .emplace(
                                activity.getDevice(),
                                writer.add_plane(
                                    std::format(
                                        "/device:{}:{}",
                                        m_backend.getName(),
                                        activity.getDevice()
                                    ),
                                    static_cast<std::int64_t>(planes.size())
                                )
                            )
                            .first;
            }

            const auto KEY =
                std::tuple{activity.getDevice(), activity.getQueue(), activity.getKind()};
            auto line = lines.find(KEY);
            if (line == lines.end()) {
                line = lines
                           .emplace(
                               KEY,
                               writer.add_line(
                                   plane->second,
                                   std::format(
                                       "queue {} {}",
                                       activity.getQueue(),
                                       kind_name(activity.getKind())
                                   ),
                                   activity.getStartWallNs()
                               )
                           )
                           .first;
            }
            writer.add_event(
                plane->second,
                line->second,
                activity.getName(),
                activity.getStartWallNs(),
                activity.getDurationNs(),
                activity.getBytes()
            );
        }

        if (planes.empty()) {
            writer.add_warning(
                std::format(
                    "{} reported no activity: start it before the runtime initialises",
                    m_backend.getName()
                )
            );
        }

        return writer.serialize();
    }

    const ProfilerHostOps& m_ops;
    ProfileBackend& m_backend;
    std::string m_device_type;
    ProfilerStatus m_status;
};

} // namespace aten_xpu

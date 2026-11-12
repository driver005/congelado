module;

#include "docs/aten_xpu/extern/profiler/xspace_writer.h"
#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

#include <sycl/sycl.hpp>
#include <unistd.h>

export module aten_xpu_profiler_sycl:profiler;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;
import cc_ice_extern_profiler_builder;
import aten_xpu_intern;

export namespace aten_xpu {

// Profiler for the SYCL backend. Works on every device whose queues report command timestamps
// (aspect::queue_profiling): Level Zero, OpenCL, CUDA and HIP backends alike.
class SyclProfiler : public ice::builder::Profiler
{
public:
    explicit SyclProfiler(const SyclOpsTable& ops) noexcept :
        ice::builder::Profiler{ops.getStatusOps(), ops.getStringOps()},
        m_ops{ops},
        m_status{ops}
    {
    }

    ~SyclProfiler() override = default;
    SyclProfiler(const SyclProfiler&) = delete;
    SyclProfiler& operator=(const SyclProfiler&) = delete;
    SyclProfiler(SyclProfiler&&) = delete;
    SyclProfiler& operator=(SyclProfiler&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "aten_xpu");
    }

    void get_device_type(const ice::sonic::String& out_device_type) noexcept override
    {
        m_status.copy_into(out_device_type, "XPU");
    }

    void start(const ice::sonic::Status& out_status) noexcept override
    {
        static_cast<void>(out_status);
        auto& collector = SyclOpsTable::getInstance().getProfileCollector();
        static_cast<void>(collector.drain());
        static_cast<void>(collector.drainMemory());
        collector.setEnabled(true);
    }

    void stop(const ice::sonic::Status& out_status) noexcept override
    {
        static_cast<void>(out_status);
        SyclOpsTable::getInstance().getProfileCollector().setEnabled(false);
    }

    void collect_data_xspace(
        ::TF_Tensor** out_data,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            const std::string BYTES = build_xspace();
            const std::array<std::int64_t, 1> DIMS{static_cast<std::int64_t>(BYTES.size())};

            SyclTensorFactory factory{m_ops};
            ::TF_Tensor* handle = factory.empty(
                DIMS,
                ice::sonic::DataTypeEnum::k_uint8,
                0,
                ice::sonic::MemorySpace::k_unified
            );
            auto& tensor = static_cast<SyclTensor&>(SyclTensor::from_handle(handle));
            if (tensor.getData() == nullptr) {
                m_status.fail(
                    out_status,
                    ice::sonic::Code::k_resource_exhausted,
                    "cannot allocate the XSpace output tensor"
                );
                return;
            }

            std::memcpy(tensor.getData(), BYTES.data(), BYTES.size());
            *out_data = handle;
        } catch (const std::exception& error) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error.what());
        }
    }

private:
    // Identifies one line of a plane: a queue and what it ran.
    using LineKey = std::tuple<int, int, SyclProfileActivity::Kind>;

    static std::string_view kind_name(SyclProfileActivity::Kind kind) noexcept
    {
        switch (kind) {
            case SyclProfileActivity::Kind::COPY:
                return "copy";
            case SyclProfileActivity::Kind::HOST:
                return "host";
            case SyclProfileActivity::Kind::COMPUTE:
                break;
        }

        return "compute";
    }

    // One XPlane per SYCL device, one XLine per queue and kind, plus an allocator line. Device
    // clocks are private to each queue's device, so every queue is shifted to the wall clock at
    // its first submission. An activity whose queue had no profiling is kept at its submission
    // time with zero duration, so the timeline still shows what ran.
    std::string build_xspace()
    {
        auto& collector = SyclOpsTable::getInstance().getProfileCollector();
        const auto ACTIVITIES = collector.drain();
        const auto MEMORY_EVENTS = collector.drainMemory();
        const auto DEVICES = collector.getDevices();

        XSpaceWriter writer;
        std::array<char, 256> hostname{};
        if (gethostname(hostname.data(), hostname.size() - 1) == 0) {
            writer.add_hostname(hostname.data());
        }

        std::vector<std::size_t> planes;
        for (std::size_t index = 0; index < DEVICES.size(); ++index) {
            planes.push_back(writer.add_plane(
                std::format("/device:XPU:{}", index),
                static_cast<std::int64_t>(index)
            ));
            if (!SyclProfileCollector::supportsDevice(DEVICES.at(index))) {
                writer.add_warning(
                    std::format(
                        "{} has no queue_profiling aspect: no device timestamps",
                        DEVICES.at(index).get_info<sycl::info::device::name>()
                    )
                );
            }
        }

        std::map<std::pair<int, int>, std::int64_t> offsets;
        std::map<LineKey, std::size_t> lines;
        std::size_t untimed = 0;
        std::size_t skipped = 0;
        for (const SyclProfileActivity& activity: ACTIVITIES) {
            const auto DEVICE = activity.getDeviceIndex();
            if (DEVICE < 0 || static_cast<std::size_t>(DEVICE) >= planes.size()) {
                ++skipped;
                continue;
            }

            std::int64_t start_ns = activity.getHostSubmitNs();
            std::int64_t duration_ns = 0;
            try {
                activity.getEvent().wait();
                const auto SUBMIT = static_cast<std::int64_t>(
                    activity.getEvent()
                        .get_profiling_info<sycl::info::event_profiling::command_submit>()
                );
                const auto START = static_cast<std::int64_t>(
                    activity.getEvent()
                        .get_profiling_info<sycl::info::event_profiling::command_start>()
                );
                const auto END = static_cast<std::int64_t>(
                    activity.getEvent()
                        .get_profiling_info<sycl::info::event_profiling::command_end>()
                );

                const auto OFFSET = offsets
                                        .try_emplace(
                                            {DEVICE, activity.getQueueId()},
                                            activity.getHostSubmitNs() - SUBMIT
                                        )
                                        .first->second;
                start_ns = START + OFFSET;
                duration_ns = END - START;
            } catch (const sycl::exception&) {
                ++untimed;
            }

            const LineKey KEY{DEVICE, activity.getQueueId(), activity.getKind()};
            const auto PLANE = planes.at(static_cast<std::size_t>(DEVICE));
            auto line = lines.find(KEY);
            if (line == lines.end()) {
                line = lines
                           .emplace(
                               KEY,
                               writer.add_line(
                                   PLANE,
                                   std::format(
                                       "queue {} {}",
                                       activity.getQueueId(),
                                       kind_name(activity.getKind())
                                   ),
                                   start_ns
                               )
                           )
                           .first;
            }
            writer.add_event(
                PLANE,
                line->second,
                activity.getName(),
                start_ns,
                duration_ns,
                activity.getBytes()
            );
        }

        // Allocator actions carry host time already; an unknown device falls back to plane 0.
        std::map<std::size_t, std::size_t> memory_lines;
        for (const SyclProfileMemoryEvent& event: MEMORY_EVENTS) {
            if (planes.empty()) {
                ++skipped;
                continue;
            }

            const auto PLANE = event.getDeviceIndex() < 0
                                   ? std::size_t{0}
                                   : static_cast<std::size_t>(event.getDeviceIndex());
            if (PLANE >= planes.size()) {
                ++skipped;
                continue;
            }

            auto line = memory_lines.find(PLANE);
            if (line == memory_lines.end()) {
                line = memory_lines
                           .emplace(
                               PLANE,
                               writer.add_line(planes.at(PLANE), "allocator", event.getWallNs())
                           )
                           .first;
            }
            writer.add_event(
                planes.at(PLANE),
                line->second,
                event.getAction(),
                event.getWallNs(),
                0,
                event.getBytes()
            );
        }

        if (untimed != 0) {
            writer.add_warning(
                std::format(
                    "{} activities have no device timestamps: their queues were created without "
                    "profiling (set ATEN_XPU_PROFILING=1 before the plugin loads); they are shown "
                    "at submission time",
                    untimed
                )
            );
        }
        if (skipped != 0) {
            writer.add_warning(std::format("{} events skipped: unknown device", skipped));
        }

        return writer.serialize();
    }

    const SyclOpsTable& m_ops;
    SyclStatus m_status;
};

} // namespace aten_xpu

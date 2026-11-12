export module aten_xpu_profiler_common:backend;

import std;
import :device_activity;

export namespace aten_xpu {

// A vendor tracer (Intel PTI, NVIDIA CUPTI, AMD rocprofiler) that reports what the device ran,
// including work the plugin never submitted itself.
class ProfileBackend
{
public:
    ProfileBackend() = default;
    virtual ~ProfileBackend() = default;
    ProfileBackend(const ProfileBackend&) = delete;
    ProfileBackend& operator=(const ProfileBackend&) = delete;
    ProfileBackend(ProfileBackend&&) = delete;
    ProfileBackend& operator=(ProfileBackend&&) = delete;

    [[nodiscard]] virtual std::string_view getName() const noexcept = 0;

    // False when the vendor library or its tracing API is not usable on this machine.
    virtual bool start() = 0;
    virtual void stop() = 0;

    // Moves records the vendor library still buffers into the store.
    virtual void flush() {}

    [[nodiscard]] std::vector<ProfileDeviceActivity> drain()
    {
        flush();
        const std::lock_guard<std::mutex> lock{m_mutex};

        return std::exchange(m_activities, {});
    }

protected:
    [[nodiscard]] static std::int64_t wallNowNs() noexcept
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    // Vendor callbacks run on library threads.
    void add(ProfileDeviceActivity activity)
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        m_activities.push_back(std::move(activity));
    }

private:
    std::mutex m_mutex;
    std::vector<ProfileDeviceActivity> m_activities;
};

} // namespace aten_xpu

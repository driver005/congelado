module;

// NVIDIA CUPTI activity API (cupti_activity.h, CUPTI_API_VERSION 28 / CUDA 12.9: kernel records
// are CUpti_ActivityKernel9, copies Memcpy6, fills Memset4). The header is optional at build time
// and libcupti.so is optional at run time.
#if __has_include(<cupti.h>)
    #include <cupti.h>
    #define ATEN_XPU_HAS_CUPTI 1
#endif

#include <cstdlib>

export module aten_xpu_profiler_cupti:backend;

import std;
import aten_xpu_profiler_common;

export namespace aten_xpu {

class CuptiBackend : public ProfileBackend
{
public:
    static CuptiBackend& instance()
    {
        static CuptiBackend backend;
        return backend;
    }

    [[nodiscard]] std::string_view getName() const noexcept override
    {
        return "cupti";
    }

#if ATEN_XPU_HAS_CUPTI
    bool start() override
    {
        static constexpr std::array<std::string_view, 2> NAMES{"libcupti.so", "libcupti.so.12"};
        if (!m_library.open(NAMES, false)) {
            return false;
        }

        m_register = m_library.symbol<decltype(&cuptiActivityRegisterCallbacks)>(
            "cuptiActivityRegisterCallbacks"
        );
        m_enable = m_library.symbol<decltype(&cuptiActivityEnable)>("cuptiActivityEnable");
        m_disable = m_library.symbol<decltype(&cuptiActivityDisable)>("cuptiActivityDisable");
        m_flush = m_library.symbol<decltype(&cuptiActivityFlushAll)>("cuptiActivityFlushAll");
        m_next =
            m_library.symbol<decltype(&cuptiActivityGetNextRecord)>("cuptiActivityGetNextRecord");
        m_timestamp = m_library.symbol<decltype(&cuptiGetTimestamp)>("cuptiGetTimestamp");
        if (m_register == nullptr || m_enable == nullptr || m_disable == nullptr ||
            m_flush == nullptr || m_next == nullptr || m_timestamp == nullptr) {
            return false;
        }

        // Activity timestamps use CUPTI's clock; shift them onto the wall clock once.
        std::uint64_t now = 0;
        if (m_timestamp(&now) != CUPTI_SUCCESS) {
            return false;
        }
        m_offset_ns = wallNowNs() - static_cast<std::int64_t>(now);
        if (m_register(&buffer_requested, &buffer_completed) != CUPTI_SUCCESS) {
            return false;
        }

        bool any = false;
        for (const CUpti_ActivityKind kind:
             {CUPTI_ACTIVITY_KIND_CONCURRENT_KERNEL,
              CUPTI_ACTIVITY_KIND_MEMCPY,
              CUPTI_ACTIVITY_KIND_MEMSET}) {
            any = m_enable(kind) == CUPTI_SUCCESS || any;
        }

        return any;
    }

    void stop() override
    {
        if (m_disable == nullptr) {
            return;
        }

        for (const CUpti_ActivityKind kind:
             {CUPTI_ACTIVITY_KIND_CONCURRENT_KERNEL,
              CUPTI_ACTIVITY_KIND_MEMCPY,
              CUPTI_ACTIVITY_KIND_MEMSET}) {
            static_cast<void>(m_disable(kind));
        }
        flush();
    }

    void flush() override
    {
        if (m_flush != nullptr) {
            static_cast<void>(m_flush(0));
        }
    }
#else
    bool start() override
    {
        return false;
    }

    void stop() override {}
#endif

private:
    CuptiBackend() = default;

#if ATEN_XPU_HAS_CUPTI
    static constexpr std::size_t BUFFER_BYTES = std::size_t{8} << 20;

    static void CUPTIAPI buffer_requested(uint8_t** buffer, std::size_t* size, std::size_t* records)
    {
        *size = BUFFER_BYTES;
        *buffer = static_cast<uint8_t*>(std::aligned_alloc(8, BUFFER_BYTES));
        *records = 0;
    }

    static void CUPTIAPI buffer_completed(
        CUcontext context,
        uint32_t stream,
        uint8_t* buffer,
        std::size_t size,
        std::size_t valid
    )
    {
        static_cast<void>(context);
        static_cast<void>(stream);
        static_cast<void>(size);
        if (buffer == nullptr) {
            return;
        }
        if (valid != 0) {
            instance().parse(buffer, valid);
        }
        std::free(buffer);
    }

    void parse(uint8_t* buffer, std::size_t valid)
    {
        CUpti_Activity* record = nullptr;
        while (m_next(buffer, valid, &record) == CUPTI_SUCCESS) {
            switch (record->kind) {
                case CUPTI_ACTIVITY_KIND_CONCURRENT_KERNEL:
                case CUPTI_ACTIVITY_KIND_KERNEL:
                    {
                        const auto* kernel = reinterpret_cast<const CUpti_ActivityKernel9*>(record);
                        keep(
                            kernel->deviceId,
                            kernel->name,
                            ProfileKind::COMPUTE,
                            kernel->streamId,
                            kernel->start,
                            kernel->end,
                            0
                        );
                        break;
                    }
                case CUPTI_ACTIVITY_KIND_MEMCPY:
                    {
                        const auto* copy = reinterpret_cast<const CUpti_ActivityMemcpy6*>(record);
                        keep(
                            copy->deviceId,
                            "memcpy",
                            ProfileKind::COPY,
                            copy->streamId,
                            copy->start,
                            copy->end,
                            copy->bytes
                        );
                        break;
                    }
                case CUPTI_ACTIVITY_KIND_MEMSET:
                    {
                        const auto* fill = reinterpret_cast<const CUpti_ActivityMemset4*>(record);
                        keep(
                            fill->deviceId,
                            "memset",
                            ProfileKind::COPY,
                            fill->streamId,
                            fill->start,
                            fill->end,
                            fill->bytes
                        );
                        break;
                    }
                default:
                    break;
            }
        }
    }

    void keep(
        uint32_t device,
        const char* name,
        ProfileKind kind,
        std::uint64_t queue,
        std::uint64_t start,
        std::uint64_t end,
        std::uint64_t bytes
    )
    {
        if (end < start) {
            return;
        }

        add(ProfileDeviceActivity{
            std::format("cuda:{}", device),
            name == nullptr ? "cupti_activity" : name,
            kind,
            queue,
            static_cast<std::int64_t>(start) + m_offset_ns,
            static_cast<std::int64_t>(end - start),
            bytes
        });
    }

    DynamicLibrary m_library;
    decltype(&cuptiActivityRegisterCallbacks) m_register{nullptr};
    decltype(&cuptiActivityEnable) m_enable{nullptr};
    decltype(&cuptiActivityDisable) m_disable{nullptr};
    decltype(&cuptiActivityFlushAll) m_flush{nullptr};
    decltype(&cuptiActivityGetNextRecord) m_next{nullptr};
    decltype(&cuptiGetTimestamp) m_timestamp{nullptr};
    std::int64_t m_offset_ns{0};
#endif
};

} // namespace aten_xpu

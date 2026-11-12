module;

// Intel PTI view API (intel/pti-gpu, sdk/include/pti/pti_view.h). The header is optional at build
// time and libpti_view.so is optional at run time.
#if __has_include(<pti/pti_view.h>)
    #include <pti/pti_view.h>
    #define ATEN_XPU_HAS_PTI 1
#endif

#include <cstdlib>
#include <cstring>

export module aten_xpu_profiler_pti:backend;

import std;
import aten_xpu_profiler_common;

export namespace aten_xpu {

class PtiBackend : public ProfileBackend
{
public:
    static PtiBackend& instance()
    {
        static PtiBackend backend;
        return backend;
    }

    [[nodiscard]] std::string_view getName() const noexcept override
    {
        return "pti";
    }

#if ATEN_XPU_HAS_PTI
    bool start() override
    {
        static constexpr std::array<std::string_view, 2> NAMES{
            "libpti_view.so",
            "libpti_view.so.0"
        };
        if (!m_library.open(NAMES, false)) {
            return false;
        }

        m_set_callbacks = m_library.symbol<decltype(&ptiViewSetCallbacks)>("ptiViewSetCallbacks");
        m_enable = m_library.symbol<decltype(&ptiViewEnable)>("ptiViewEnable");
        m_disable = m_library.symbol<decltype(&ptiViewDisable)>("ptiViewDisable");
        m_flush = m_library.symbol<decltype(&ptiFlushAllViews)>("ptiFlushAllViews");
        m_next_record = m_library.symbol<decltype(&ptiViewGetNextRecord)>("ptiViewGetNextRecord");
        m_timestamp = m_library.symbol<decltype(&ptiViewGetTimestamp)>("ptiViewGetTimestamp");
        if (m_set_callbacks == nullptr || m_enable == nullptr || m_disable == nullptr ||
            m_flush == nullptr || m_next_record == nullptr || m_timestamp == nullptr) {
            return false;
        }

        // View records use PTI's host clock; shift them onto the wall clock once.
        m_offset_ns = wallNowNs() - static_cast<std::int64_t>(m_timestamp());
        if (m_set_callbacks(&buffer_requested, &buffer_completed) != PTI_SUCCESS) {
            return false;
        }

        bool any = false;
        for (const pti_view_kind kind:
             {PTI_VIEW_DEVICE_GPU_KERNEL,
              PTI_VIEW_DEVICE_GPU_MEM_COPY,
              PTI_VIEW_DEVICE_GPU_MEM_FILL}) {
            any = m_enable(kind) == PTI_SUCCESS || any;
        }

        return any;
    }

    void stop() override
    {
        if (m_disable == nullptr) {
            return;
        }

        for (const pti_view_kind kind:
             {PTI_VIEW_DEVICE_GPU_KERNEL,
              PTI_VIEW_DEVICE_GPU_MEM_COPY,
              PTI_VIEW_DEVICE_GPU_MEM_FILL}) {
            static_cast<void>(m_disable(kind));
        }
        flush();
    }

    void flush() override
    {
        if (m_flush != nullptr) {
            static_cast<void>(m_flush());
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
    PtiBackend() = default;

#if ATEN_XPU_HAS_PTI
    static constexpr std::size_t BUFFER_BYTES = std::size_t{8} << 20;

    static void buffer_requested(unsigned char** buffer, std::size_t* size)
    {
        *size = BUFFER_BYTES;
        *buffer = static_cast<unsigned char*>(std::aligned_alloc(8, BUFFER_BYTES));
    }

    static void buffer_completed(unsigned char* buffer, std::size_t size, std::size_t used)
    {
        static_cast<void>(size);
        if (buffer == nullptr) {
            return;
        }
        if (used != 0) {
            instance().parse(buffer, used);
        }
        std::free(buffer);
    }

    // Kernel, copy and fill records share the leading fields read here.
    void parse(unsigned char* buffer, std::size_t used)
    {
        pti_view_record_base* record = nullptr;
        while (m_next_record(buffer, used, &record) == PTI_SUCCESS) {
            switch (record->_view_kind) {
                case PTI_VIEW_DEVICE_GPU_KERNEL:
                    {
                        const auto* kernel =
                            reinterpret_cast<const pti_view_record_kernel*>(record);
                        keep(
                            kernel->_pci_address,
                            kernel->_name,
                            ProfileKind::COMPUTE,
                            kernel->_sycl_queue_id,
                            kernel->_start_timestamp,
                            kernel->_end_timestamp,
                            0
                        );
                        break;
                    }
                case PTI_VIEW_DEVICE_GPU_MEM_COPY:
                    {
                        const auto* copy =
                            reinterpret_cast<const pti_view_record_memory_copy*>(record);
                        keep(
                            copy->_pci_address,
                            copy->_name,
                            ProfileKind::COPY,
                            copy->_sycl_queue_id,
                            copy->_start_timestamp,
                            copy->_end_timestamp,
                            copy->_bytes
                        );
                        break;
                    }
                case PTI_VIEW_DEVICE_GPU_MEM_FILL:
                    {
                        const auto* fill =
                            reinterpret_cast<const pti_view_record_memory_fill*>(record);
                        keep(
                            fill->_pci_address,
                            fill->_name,
                            ProfileKind::COPY,
                            fill->_sycl_queue_id,
                            fill->_start_timestamp,
                            fill->_end_timestamp,
                            fill->_bytes
                        );
                        break;
                    }
                default:
                    break;
            }
        }
    }

    // PTI reports 0 for a timestamp it could not determine; such records are dropped.
    void keep(
        const char* pci_address,
        const char* name,
        ProfileKind kind,
        std::uint64_t queue,
        std::uint64_t start,
        std::uint64_t end,
        std::uint64_t bytes
    )
    {
        if (start == 0 || end < start) {
            return;
        }

        add(ProfileDeviceActivity{
            std::string{pci_address, ::strnlen(pci_address, PTI_MAX_PCI_ADDRESS_SIZE)},
            name == nullptr ? "pti_activity" : name,
            kind,
            queue,
            static_cast<std::int64_t>(start) + m_offset_ns,
            static_cast<std::int64_t>(end - start),
            bytes
        });
    }

    DynamicLibrary m_library;
    decltype(&ptiViewSetCallbacks) m_set_callbacks{nullptr};
    decltype(&ptiViewEnable) m_enable{nullptr};
    decltype(&ptiViewDisable) m_disable{nullptr};
    decltype(&ptiFlushAllViews) m_flush{nullptr};
    decltype(&ptiViewGetNextRecord) m_next_record{nullptr};
    decltype(&ptiViewGetTimestamp) m_timestamp{nullptr};
    std::int64_t m_offset_ns{0};
#endif
};

} // namespace aten_xpu

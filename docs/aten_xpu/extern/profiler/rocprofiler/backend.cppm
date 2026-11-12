module;

// AMD rocprofiler-sdk (ROCm/rocm-systems, projects/rocprofiler-sdk). The header is optional at
// build time. The library is never loaded by the plugin: rocprofiler-sdk finds the exported
// `rocprofiler_configure` symbol (plugin.cc) when the HSA runtime starts, then calls initialize.
#if __has_include(<rocprofiler-sdk/rocprofiler.h>)
    #include <rocprofiler-sdk/registration.h>
    #include <rocprofiler-sdk/rocprofiler.h>
    #define ATEN_XPU_HAS_ROCPROFILER 1
#endif

export module aten_xpu_profiler_rocprofiler:backend;

import std;
import aten_xpu_profiler_common;

export namespace aten_xpu {

class RocprofilerBackend : public ProfileBackend
{
public:
    static RocprofilerBackend& instance()
    {
        static RocprofilerBackend backend;
        return backend;
    }

    [[nodiscard]] std::string_view getName() const noexcept override
    {
        return "rocprofiler";
    }

#if ATEN_XPU_HAS_ROCPROFILER
    // Exported to rocprofiler-sdk as `rocprofiler_configure` by plugin.cc.
    static rocprofiler_tool_configure_result_t* configure(
        uint32_t version,
        const char* runtime_version,
        uint32_t priority,
        rocprofiler_client_id_t* client_id
    )
    {
        static_cast<void>(version);
        static_cast<void>(runtime_version);
        static_cast<void>(priority);
        client_id->name = "aten_xpu";

        static rocprofiler_tool_configure_result_t
            result{sizeof(rocprofiler_tool_configure_result_t), &initialize, &finalize, nullptr};

        return &result;
    }

    // False unless rocprofiler-sdk called configure(), which only happens when the HSA runtime
    // starts after this plugin was loaded.
    bool start() override
    {
        if (!m_ready.load() || m_start_context == nullptr) {
            return false;
        }

        rocprofiler_timestamp_t now = 0;
        if (m_get_timestamp(&now) != ROCPROFILER_STATUS_SUCCESS) {
            return false;
        }
        m_offset_ns = wallNowNs() - static_cast<std::int64_t>(now);

        return m_start_context(m_context) == ROCPROFILER_STATUS_SUCCESS;
    }

    void stop() override
    {
        if (m_ready.load() && m_stop_context != nullptr) {
            static_cast<void>(m_stop_context(m_context));
            flush();
        }
    }

    void flush() override
    {
        if (m_ready.load() && m_flush_buffer != nullptr) {
            static_cast<void>(m_flush_buffer(m_buffer));
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
    RocprofilerBackend() = default;

#if ATEN_XPU_HAS_ROCPROFILER
    static int initialize(rocprofiler_client_finalize_t finalize_function, void* tool_data)
    {
        static_cast<void>(finalize_function);
        static_cast<void>(tool_data);

        return instance().create_context() ? 0 : -1;
    }

    static void finalize(void* tool_data)
    {
        static_cast<void>(tool_data);
    }

    // Called inside rocprofiler's initialization: the context is created but not started.
    bool create_context()
    {
        static constexpr std::array<std::string_view, 2> NAMES{
            "librocprofiler-sdk.so",
            "librocprofiler-sdk.so.1"
        };
        if (!m_library.open(NAMES, true)) {
            return false;
        }

        auto make_context =
            m_library.symbol<decltype(&rocprofiler_create_context)>("rocprofiler_create_context");
        auto create_buffer =
            m_library.symbol<decltype(&rocprofiler_create_buffer)>("rocprofiler_create_buffer");
        auto buffer_service =
            m_library.symbol<decltype(&rocprofiler_configure_buffer_tracing_service)>(
                "rocprofiler_configure_buffer_tracing_service"
            );
        auto callback_service =
            m_library.symbol<decltype(&rocprofiler_configure_callback_tracing_service)>(
                "rocprofiler_configure_callback_tracing_service"
            );
        m_start_context =
            m_library.symbol<decltype(&rocprofiler_start_context)>("rocprofiler_start_context");
        m_stop_context =
            m_library.symbol<decltype(&rocprofiler_stop_context)>("rocprofiler_stop_context");
        m_flush_buffer =
            m_library.symbol<decltype(&rocprofiler_flush_buffer)>("rocprofiler_flush_buffer");
        m_get_timestamp =
            m_library.symbol<decltype(&rocprofiler_get_timestamp)>("rocprofiler_get_timestamp");
        if (make_context == nullptr || create_buffer == nullptr || buffer_service == nullptr ||
            callback_service == nullptr || m_start_context == nullptr ||
            m_stop_context == nullptr || m_flush_buffer == nullptr || m_get_timestamp == nullptr) {
            return false;
        }

        constexpr std::size_t BUFFER_BYTES = std::size_t{256} << 10;
        constexpr std::size_t WATERMARK_BYTES = std::size_t{192} << 10;
        if (make_context(&m_context) != ROCPROFILER_STATUS_SUCCESS ||
            create_buffer(
                m_context,
                BUFFER_BYTES,
                WATERMARK_BYTES,
                ROCPROFILER_BUFFER_POLICY_LOSSLESS,
                &buffer_completed,
                nullptr,
                &m_buffer
            ) != ROCPROFILER_STATUS_SUCCESS) {
            return false;
        }

        // Kernel names arrive once, when a code object loads; dispatch records carry only an id.
        const rocprofiler_tracing_operation_t SYMBOL_OPERATION =
            ROCPROFILER_CODE_OBJECT_DEVICE_KERNEL_SYMBOL_REGISTER;
        bool configured = callback_service(
                              m_context,
                              ROCPROFILER_CALLBACK_TRACING_CODE_OBJECT,
                              &SYMBOL_OPERATION,
                              1,
                              &code_object_callback,
                              nullptr
                          ) == ROCPROFILER_STATUS_SUCCESS;
        for (const rocprofiler_buffer_tracing_kind_t kind:
             {ROCPROFILER_BUFFER_TRACING_KERNEL_DISPATCH, ROCPROFILER_BUFFER_TRACING_MEMORY_COPY}) {
            configured = buffer_service(m_context, kind, nullptr, 0, m_buffer) ==
                             ROCPROFILER_STATUS_SUCCESS &&
                         configured;
        }
        m_ready.store(configured);

        return configured;
    }

    static void code_object_callback(
        rocprofiler_callback_tracing_record_t record,
        rocprofiler_user_data_t* user_data,
        void* callback_data
    )
    {
        static_cast<void>(user_data);
        static_cast<void>(callback_data);
        if (record.kind != ROCPROFILER_CALLBACK_TRACING_CODE_OBJECT ||
            record.operation != ROCPROFILER_CODE_OBJECT_DEVICE_KERNEL_SYMBOL_REGISTER ||
            record.phase != ROCPROFILER_CALLBACK_PHASE_LOAD) {
            return;
        }

        const auto* symbol = static_cast<
            const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t*>(
            record.payload
        );
        instance().remember_kernel(symbol->kernel_id, symbol->kernel_name);
    }

    static void buffer_completed(
        rocprofiler_context_id_t context,
        rocprofiler_buffer_id_t buffer,
        rocprofiler_record_header_t** headers,
        std::size_t count,
        void* data,
        uint64_t dropped
    )
    {
        static_cast<void>(context);
        static_cast<void>(buffer);
        static_cast<void>(data);
        static_cast<void>(dropped);
        for (const rocprofiler_record_header_t* header: std::span{headers, count}) {
            if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING) {
                instance().parse(*header);
            }
        }
    }

    void remember_kernel(std::uint64_t kernel_id, const char* name)
    {
        const std::lock_guard<std::mutex> lock{m_names_mutex};
        m_kernel_names.insert_or_assign(kernel_id, name == nullptr ? std::string{} : name);
    }

    [[nodiscard]] std::string kernel_name(std::uint64_t kernel_id)
    {
        const std::lock_guard<std::mutex> lock{m_names_mutex};
        const auto found = m_kernel_names.find(kernel_id);

        return found == m_kernel_names.end() ? std::format("kernel {}", kernel_id) : found->second;
    }

    void parse(const rocprofiler_record_header_t& header)
    {
        if (header.kind == ROCPROFILER_BUFFER_TRACING_KERNEL_DISPATCH) {
            const auto* dispatch =
                static_cast<const rocprofiler_buffer_tracing_kernel_dispatch_record_t*>(
                    header.payload
                );
            keep(
                dispatch->dispatch_info.agent_id.handle,
                kernel_name(dispatch->dispatch_info.kernel_id),
                ProfileKind::COMPUTE,
                dispatch->dispatch_info.queue_id.handle,
                dispatch->start_timestamp,
                dispatch->end_timestamp,
                0
            );
        } else if (header.kind == ROCPROFILER_BUFFER_TRACING_MEMORY_COPY) {
            const auto* copy =
                static_cast<const rocprofiler_buffer_tracing_memory_copy_record_t*>(header.payload);
            keep(
                copy->dst_agent_id.handle,
                "memcpy",
                ProfileKind::COPY,
                0,
                copy->start_timestamp,
                copy->end_timestamp,
                copy->bytes
            );
        }
    }

    void keep(
        std::uint64_t agent,
        std::string name,
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
            std::format("agent:{}", agent),
            std::move(name),
            kind,
            queue,
            static_cast<std::int64_t>(start) + m_offset_ns,
            static_cast<std::int64_t>(end - start),
            bytes
        });
    }

    DynamicLibrary m_library;
    decltype(&rocprofiler_start_context) m_start_context{nullptr};
    decltype(&rocprofiler_stop_context) m_stop_context{nullptr};
    decltype(&rocprofiler_flush_buffer) m_flush_buffer{nullptr};
    decltype(&rocprofiler_get_timestamp) m_get_timestamp{nullptr};
    rocprofiler_context_id_t m_context{};
    rocprofiler_buffer_id_t m_buffer{};
    std::atomic<bool> m_ready{false};
    std::int64_t m_offset_ns{0};
    std::mutex m_names_mutex;
    std::unordered_map<std::uint64_t, std::string> m_kernel_names;
#endif
};

} // namespace aten_xpu

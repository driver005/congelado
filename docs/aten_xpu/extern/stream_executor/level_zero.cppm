module;

#include <dlfcn.h>
#include <level_zero/ze_api.h>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:level_zero;

import std;

export namespace aten_xpu {

class SyclLevelZero
{
public:
    SyclLevelZero() = default;
    ~SyclLevelZero()
    {

        if (m_library != nullptr) {
            dlclose(m_library);
        }

    }

    SyclLevelZero(const SyclLevelZero&) = delete;
    SyclLevelZero& operator=(const SyclLevelZero&) = delete;
    SyclLevelZero(SyclLevelZero&&) = delete;
    SyclLevelZero& operator=(SyclLevelZero&&) = delete;

    static SyclLevelZero& getInstance()
    {

        static SyclLevelZero instance;
        return instance;

    }

    std::optional<ze_device_handle_t>
    query_allocation_device(const sycl::context& context, const void* pointer)
    {

        if (!load()) {
            return std::nullopt;
        }

        auto native_context = sycl::get_native<sycl::backend::ext_oneapi_level_zero>(context);
        ze_memory_allocation_properties_t properties{
            .stype = ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES
        };
        ze_device_handle_t device = nullptr;
        if (m_mem_get_alloc_properties(native_context, pointer, &properties, &device) !=
            ZE_RESULT_SUCCESS)
        {
            return std::nullopt;
        }
        return device;

    }

    std::optional<ze_kernel_properties_t> query_kernel_properties(ze_kernel_handle_t kernel)
    {

        if (!load()) {
            return std::nullopt;
        }

        ze_kernel_properties_t properties{.stype = ZE_STRUCTURE_TYPE_KERNEL_PROPERTIES};
        if (m_kernel_get_properties(kernel, &properties) != ZE_RESULT_SUCCESS) {
            return std::nullopt;
        }
        return properties;

    }

    std::expected<ze_kernel_handle_t, std::string> create_kernel(
        ze_context_handle_t context,
        ze_device_handle_t device,
        std::span<const std::uint8_t> spirv,
        const char* kernel_name
    )
    {

        if (!load()) {
            return std::unexpected{std::string{"libze_loader.so not available"}};
        }

        ze_module_desc_t module_description{
            .stype = ZE_STRUCTURE_TYPE_MODULE_DESC,
            .format = ZE_MODULE_FORMAT_IL_SPIRV,
            .inputSize = spirv.size(),
            .pInputModule = spirv.data()
        };
        ze_module_handle_t module_handle = nullptr;
        ze_module_build_log_handle_t build_log = nullptr;
        if (m_module_create(context, device, &module_description, &module_handle, &build_log) !=
            ZE_RESULT_SUCCESS)
        {
            return std::unexpected{read_build_log(build_log)};
        }
        m_module_build_log_destroy(build_log);

        ze_kernel_desc_t kernel_description{
            .stype = ZE_STRUCTURE_TYPE_KERNEL_DESC,
            .pKernelName = kernel_name
        };
        ze_kernel_handle_t kernel = nullptr;
        if (m_kernel_create(module_handle, &kernel_description, &kernel) != ZE_RESULT_SUCCESS) {
            return std::unexpected{std::string{"zeKernelCreate failed"}};
        }
        return kernel;

    }

private:
    bool load()
    {

        if (m_library != nullptr) {
            return true;
        }

        m_library = dlopen("libze_loader.so", RTLD_NOW | RTLD_LOCAL);
        if (m_library == nullptr) {
            return false;
        }

        resolve(m_module_create, "zeModuleCreate");
        resolve(m_kernel_create, "zeKernelCreate");
        resolve(m_kernel_get_properties, "zeKernelGetProperties");
        resolve(m_mem_get_alloc_properties, "zeMemGetAllocProperties");
        resolve(m_module_build_log_get_string, "zeModuleBuildLogGetString");
        resolve(m_module_build_log_destroy, "zeModuleBuildLogDestroy");
        return true;

    }

    template<typename Function>
    void resolve(Function& target, const char* symbol)
    {

        target = reinterpret_cast<Function>(dlsym(m_library, symbol));

    }

    std::string read_build_log(ze_module_build_log_handle_t build_log)
    {

        std::size_t size = 0;
        m_module_build_log_get_string(build_log, &size, nullptr);
        m_scratch_log.resize(size);
        m_module_build_log_get_string(build_log, &size, m_scratch_log.data());
        m_module_build_log_destroy(build_log);
        return m_scratch_log;

    }

    void* m_library{nullptr};
    decltype(&zeModuleCreate) m_module_create{nullptr};
    decltype(&zeKernelCreate) m_kernel_create{nullptr};
    decltype(&zeKernelGetProperties) m_kernel_get_properties{nullptr};
    decltype(&zeMemGetAllocProperties) m_mem_get_alloc_properties{nullptr};
    decltype(&zeModuleBuildLogGetString) m_module_build_log_get_string{nullptr};
    decltype(&zeModuleBuildLogDestroy) m_module_build_log_destroy{nullptr};
    std::string m_scratch_log;
};

} // namespace aten_xpu

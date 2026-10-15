module;

#include "include/c/extern/stream_executor/types.h"

#include <sycl/sycl.hpp>

export module aten_xpu_intern:placement;

import std;

export namespace aten_xpu {

class SyclTensorPlacement
{
public:
    using Allocate = std::function<void*(std::size_t, TF_MemorySpace)>;
    using Deallocate = std::function<void(void*, TF_MemorySpace)>;

    void setPlacement(const sycl::context& context, const sycl::device& device, int device_index)
    {
        m_context = context;
        m_device = device;
        m_default_device_index = device_index;
    }

    void setStorageAllocator(Allocate allocate, Deallocate deallocate)
    {
        m_allocate = std::move(allocate);
        m_deallocate = std::move(deallocate);
    }

    int getDefaultDeviceIndex() const noexcept
    {
        return m_default_device_index;
    }

    bool hasPlacement() const noexcept
    {
        return m_context.has_value() && m_device.has_value();
    }

    const std::optional<sycl::context>& getContext() const noexcept
    {
        return m_context;
    }

    const std::optional<sycl::device>& getDevice() const noexcept
    {
        return m_device;
    }

    void* allocate_storage(std::size_t size, TF_MemorySpace memory_space) const
    {
        if (m_allocate) {
            return m_allocate(size, memory_space);
        }
        if (!hasPlacement()) {
            return std::malloc(size);
        }
        switch (memory_space) {
            case TF_MEMORY_SPACE_HOST_PINNED:
                return sycl::malloc_host(size, *m_context);
            case TF_MEMORY_SPACE_UNIFIED:
                return sycl::malloc_shared(size, *m_device, *m_context);
            default:
                return sycl::malloc_device(size, *m_device, *m_context);
        }
    }

    void deallocate_storage(void* storage, TF_MemorySpace memory_space) const
    {
        if (m_deallocate) {
            m_deallocate(storage, memory_space);
            return;
        }
        if (!m_context) {
            std::free(storage);
            return;
        }
        sycl::free(storage, *m_context);
    }

private:
    std::optional<sycl::context> m_context;
    std::optional<sycl::device> m_device;
    int m_default_device_index{0};
    Allocate m_allocate;
    Deallocate m_deallocate;
};

} // namespace aten_xpu

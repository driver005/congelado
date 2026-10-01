// SYCL reference plugin — TF_MemoryOps facade: owns one SyclAllocator per device.
//
// Not built by Bazel (docs/ only). See platform.cppm's file-level note: create_allocator_internal
// takes the raw TF_Allocator* out handle, same reasoning as every other create_*_internal slot.

module;

#include "include/c/extern/stream_executor/memory.h"

export module sycl_backend:memory;

import std;
import cc_ice_extern_memory_builder;
import :platform;
import :allocator;

export namespace sycl_backend {

class SyclMemory : public ice::builder::Memory
{
public:
    explicit SyclMemory(SyclPlatform& platform) noexcept :
        m_platform{platform}
    {
    }

    ~SyclMemory() override = default;
    SyclMemory(const SyclMemory&) = delete;
    SyclMemory& operator=(const SyclMemory&) = delete;
    SyclMemory(SyclMemory&&) = delete;
    SyclMemory& operator=(SyclMemory&&) = delete;

    void get_name(ice::builder::String& out_name) noexcept override
    {
        static constexpr std::string_view name = "sycl_backend";
        out_name.copy(name.data(), name.size());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_allocator_internal(
        ice::builder::Executor& executor,
        ice::builder::Device& device,
        TF_Allocator* out_allocator
    ) noexcept
    {
        (void)executor;

        auto* native_device = dynamic_cast<SyclDevice*>(&device);
        if (native_device == nullptr) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclMemory: device was not created by this plugin")
            };
        }

        auto owned = std::make_unique<SyclAllocator>(
            const_cast<sycl::context&>(m_platform.get_shared_context()),
            *native_device
        );
        out_allocator->plugin_data = owned.get();
        m_allocators.push_back(std::move(owned));
        return {};
    }

    void destroy_allocator_internal(ice::builder::Allocator& allocator) noexcept override
    {
        std::erase_if(
            m_allocators,
            [&allocator](const std::unique_ptr<SyclAllocator>& candidate)
            { return candidate.get() == &allocator; }
        );
    }

private:
    SyclPlatform& m_platform;
    std::vector<std::unique_ptr<SyclAllocator>> m_allocators;
};

} // namespace sycl_backend

module;

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

export module cc_tmp:allocator_allocator;

import std;
import cc_abi;

export {

namespace tensorflow {

// Native alias to ice::AllocatorAttributes
using AllocatorAttributes = ice::AllocatorAttributes;

struct AllocatorStats {
    int64_t num_allocs{0};
    int64_t bytes_in_use{0};
    int64_t max_bytes_in_use{0};
    int64_t max_alloc_size{0};
    int64_t bytes_limit{0};
};

class Allocator {
public:
    virtual ~Allocator() = default;

    virtual std::string Name() = 0;
    virtual void* AllocateRaw(size_t alignment, size_t num_bytes) = 0;
    virtual void* AllocateRaw(size_t alignment, size_t num_bytes, const AllocatorAttributes& /*attrs*/) {
        return AllocateRaw(alignment, num_bytes);
    }
    virtual void DeallocateRaw(void* ptr) = 0;

    virtual bool TracksAllocationSizes() const { return false; }
    virtual size_t RequestedSize(const void* /*ptr*/) const { return 0; }
    virtual size_t AllocatedSize(const void* /*ptr*/) const { return 0; }
    virtual std::optional<AllocatorStats> GetStats() { return std::nullopt; }
    virtual void ClearStats() {}
};

class CPUAllocator : public Allocator {
public:
    std::string Name() override { return "cpu"; }

    void* AllocateRaw(size_t alignment, size_t num_bytes) override {
        void* ptr = nullptr;
        if (alignment < sizeof(void*)) alignment = sizeof(void*);
        if (posix_memalign(&ptr, alignment, num_bytes) != 0) {
            return nullptr;
        }
        return ptr;
    }

    void DeallocateRaw(void* ptr) override {
        std::free(ptr);
    }
};

inline Allocator* cpu_allocator() {
    static CPUAllocator s_cpu_allocator;
    return &s_cpu_allocator;
}

} // namespace tensorflow

} // export

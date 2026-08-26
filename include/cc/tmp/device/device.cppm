module;

#include <memory>
#include <string>

export module cc_tmp:device_device;

import std;
import cc_abi;
import :allocator_allocator;

export {

    namespace tensorflow {

        class Device
        {
        public:
            explicit Device(ice::Device device) :
                m_device{std::move(device)},
                m_allocator{cpu_allocator()}
            {
            }

            virtual ~Device() = default;

            Allocator* GetAllocator(const AllocatorAttributes& /*attr*/)
            {
                return m_allocator;
            }

            ice::Device& ice_device()
            {
                return m_device;
            }

            const ice::Device& ice_device() const
            {
                return m_device;
            }

        private:
            ice::Device m_device;
            Allocator* m_allocator;
        };

    } // namespace tensorflow

} // export

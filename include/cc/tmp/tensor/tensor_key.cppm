module;

#include <cstddef>
#include <cstdint>
#include <functional>

export module cc_tmp:tensor_tensor_key;

import std;
import cc_abi;
import :tensor_tensor;

export {

    namespace tensorflow {

        class TensorKey
        {
        public:
            TensorKey() :
                m_device_id(0),
                m_tensor_ptr(nullptr)
            {
            }

            explicit TensorKey(const Tensor& tensor, int64_t device_id = 0) :
                m_device_id(device_id),
                m_tensor_ptr(tensor.buffer())
            {
            }

            bool operator==(const TensorKey& other) const
            {
                return m_device_id == other.m_device_id && m_tensor_ptr == other.m_tensor_ptr;
            }

            struct Hash
            {
                size_t operator()(const TensorKey& k) const noexcept
                {
                    size_t h1 = std::hash<int64_t>{}(k.m_device_id);
                    size_t h2 = std::hash<const void*>{}(k.m_tensor_ptr);
                    return h1 ^ (h2 << 1);
                }
            };

        private:
            int64_t m_device_id;
            const void* m_tensor_ptr;
        };

    } // namespace tensorflow

} // export

module;

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

export module cc_tmp:variant_variant;

import std;
import cc_abi;

export {

    namespace tensorflow {

        class Variant
        {
        public:
            Variant() :
                m_variant{nullptr}
            {
            }

            explicit Variant(TF_Variant* v) :
                m_variant{v}
            {
            }

            ~Variant() = default;

            Variant(const Variant&) = delete;
            Variant& operator=(const Variant&) = delete;

            Variant(Variant&& other) noexcept :
                m_variant{std::move(other.m_variant)}
            {
            }

            Variant& operator=(Variant&& other) noexcept
            {
                if (this != &other) {
                    m_variant = std::move(other.m_variant);
                }
                return *this;
            }

            bool is_empty() const
            {
                return m_variant.get_handle() == nullptr;
            }

            ice::Variant& ice_variant()
            {
                return m_variant;
            }

            const ice::Variant& ice_variant() const
            {
                return m_variant;
            }

        private:
            ice::Variant m_variant;
        };

    } // namespace tensorflow

} // export

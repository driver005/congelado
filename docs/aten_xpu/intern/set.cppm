module;

#include "include/c/intern/set.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:set;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// An ordered set of fixed size byte keys; the constructor sets the key size and keys order
// byte-wise.
class Set : public ice::builder::Set
{
public:
    Set(const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        std::size_t key_size) noexcept :
        ice::builder::Set{status_ops},
        m_failure{string_ops},
        m_key_size{key_size}
    {
    }

    ~Set() override = default;
    Set(const Set&) = delete;
    Set& operator=(const Set&) = delete;
    Set(Set&&) = delete;
    Set& operator=(Set&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void insert(const void* key, const ice::sonic::Status& out_status) noexcept override
    {
        if (m_key_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no key size");
            return;
        }

        m_items.insert(bytes_of(key));
    }

    // A set has no value, so this reports the stored key (nullptr when absent).
    void find(
        const void* key,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        const auto found = m_items.find(bytes_of(key));
        *out_value = found == m_items.end() ? nullptr : found->data();
    }

    void erase(const void* key, const ice::sonic::Status& out_status) noexcept override
    {
        if (m_items.erase(bytes_of(key)) == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, "key is not in the set");
        }
    }

    void contains(
        const void* key,
        int& out_found,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        out_found = m_items.contains(bytes_of(key)) ? 1 : 0;
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_items.size();
    }

    void for_each(TF_SetVisitor visitor, void* capture) noexcept override
    {
        for (const auto& key: m_items) {
            visitor(capture, key.data());
        }
    }

private:
    std::vector<std::byte> bytes_of(const void* data) const
    {
        const auto* first = static_cast<const std::byte*>(data);

        return std::vector<std::byte>(first, first + m_key_size);
    }

    Failure m_failure;
    std::size_t m_key_size;
    std::set<std::vector<std::byte>> m_items;
};

} // namespace aten_xpu

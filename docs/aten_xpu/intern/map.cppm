module;

#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:map;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// An ordered map of fixed size byte keys and values; the ABI has no size slots, so the constructor
// sets them. Keys order byte-wise and insert on an existing key replaces its value.
class Map : public ice::builder::Map
{
public:
    Map(const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        std::size_t key_size,
        std::size_t value_size) noexcept :
        ice::builder::Map{status_ops},
        m_failure{string_ops},
        m_key_size{key_size},
        m_value_size{value_size}
    {
    }

    ~Map() override = default;
    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;
    Map(Map&&) = delete;
    Map& operator=(Map&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void insert(
        const void* key,
        const void* value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (m_key_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no key size");
            return;
        }

        m_items.insert_or_assign(bytes_of(key, m_key_size), bytes_of(value, m_value_size));
    }

    void find(
        const void* key,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        const auto found = m_items.find(bytes_of(key, m_key_size));
        *out_value = found == m_items.end() ? nullptr : found->second.data();
    }

    void erase(const void* key, const ice::sonic::Status& out_status) noexcept override
    {
        if (m_items.erase(bytes_of(key, m_key_size)) == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, "key is not in the map");
        }
    }

    void contains(
        const void* key,
        int& out_found,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);

        out_found = m_items.contains(bytes_of(key, m_key_size)) ? 1 : 0;
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_items.size();
    }

    void for_each(TF_MapVisitor visitor, void* capture) noexcept override
    {
        for (const auto& [key, value]: m_items) {
            visitor(capture, key.data(), value.data());
        }
    }

private:
    static std::vector<std::byte> bytes_of(const void* data, std::size_t size)
    {
        const auto* first = static_cast<const std::byte*>(data);

        return std::vector<std::byte>(first, first + size);
    }

    Failure m_failure;
    std::size_t m_key_size;
    std::size_t m_value_size;
    std::map<std::vector<std::byte>, std::vector<std::byte>> m_items;
};

} // namespace aten_xpu

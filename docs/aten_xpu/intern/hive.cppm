module;

#include "include/c/intern/hive/hive.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:hive;

import std;
import cc_ice_intern_hive_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A container with stable slots: insert returns a slot that stays valid until it is erased.
class Hive : public ice::builder::Hive
{
public:
    Hive(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Hive{status_ops},
        m_failure{string_ops}
    {
    }

    ~Hive() override = default;
    Hive(const Hive&) = delete;
    Hive& operator=(const Hive&) = delete;
    Hive(Hive&&) = delete;
    Hive& operator=(Hive&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set_element_size(std::size_t element_size) noexcept override
    {
        m_element_size = element_size;
        m_items.clear();
    }

    void insert(
        const void* value,
        ::TFHiveSlot* out_slot,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (m_element_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no element size");
            return;
        }

        const auto* first = static_cast<const std::byte*>(value);
        out_slot->plugin_data =
            &m_items.emplace_back(std::vector<std::byte>(first, first + m_element_size));
    }

    void erase(::TFHiveSlot* slot, const ice::sonic::Status& out_status) noexcept override
    {
        const auto found = find(slot->plugin_data);
        if (found == m_items.end()) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, "slot is not in the hive");
            return;
        }

        m_items.erase(found);
        slot->plugin_data = nullptr;
    }

    void get(
        const ::TFHiveSlot* slot,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        const auto found = find(slot->plugin_data);
        if (found == m_items.end()) {
            m_failure.fail(out_status, ice::sonic::Code::k_not_found, "slot is not in the hive");
            return;
        }

        *out_value = found->data();
    }

    void for_each(TF_HiveVisitor visitor, void* capture) noexcept override
    {
        for (const auto& item: m_items) {
            visitor(capture, item.data());
        }
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_items.size();
    }

private:
    std::list<std::vector<std::byte>>::iterator find(const void* storage)
    {
        return std::ranges::find_if(
            m_items,
            [storage](const std::vector<std::byte>& item)
            {
                return &item == storage;
            }
        );
    }

    Failure m_failure;
    std::size_t m_element_size = 0;
    std::list<std::vector<std::byte>> m_items;
};

} // namespace aten_xpu

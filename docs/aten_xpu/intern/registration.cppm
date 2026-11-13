module;

#include "include/c/intern/registration.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:registration;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :string;

export namespace aten_xpu {

// A registry of (type, name) -> opaque pointer; the first name of a type is its default provider.
class Registration : public ice::builder::Registration
{
public:
    explicit Registration(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Registration{string_ops}
    {
    }

    ~Registration() override = default;
    Registration(const Registration&) = delete;
    Registration& operator=(const Registration&) = delete;
    Registration(Registration&&) = delete;
    Registration& operator=(Registration&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("Registration");
    }

    void register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) noexcept override
    {
        const std::string type_key{String::view_of(type)};
        const std::string name_key{String::view_of(name)};
        m_entries[type_key].insert_or_assign(name_key, value);
        m_defaults.try_emplace(type_key, name_key);
    }

    void get(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void** out_value
    ) noexcept override
    {
        *out_value = nullptr;

        const std::string type_key{String::view_of(type)};
        const auto group = m_entries.find(type_key);
        if (group == m_entries.end()) {
            return;
        }

        std::string name_key{String::view_of(name)};
        if (name_key.empty()) {
            const auto fallback = m_defaults.find(type_key);
            if (fallback == m_defaults.end()) {
                return;
            }
            name_key = fallback->second;
        }

        const auto entry = group->second.find(name_key);
        if (entry != group->second.end()) {
            *out_value = entry->second;
        }
    }

    void unregister(
        const ice::sonic::String& type,
        const ice::sonic::String& name
    ) noexcept override
    {
        const std::string type_key{String::view_of(type)};
        const std::string name_key{String::view_of(name)};
        const auto group = m_entries.find(type_key);
        if (group == m_entries.end()) {
            return;
        }

        group->second.erase(name_key);
        const auto fallback = m_defaults.find(type_key);
        if (fallback == m_defaults.end() || fallback->second != name_key) {
            return;
        }

        // The default left: the first remaining name (sorted) takes over, else the type is empty.
        if (group->second.empty()) {
            m_defaults.erase(fallback);
            m_entries.erase(group);
        } else {
            fallback->second = group->second.begin()->first;
        }
    }

    void set_default(
        const ice::sonic::String& type,
        const ice::sonic::String& name
    ) noexcept override
    {
        const std::string type_key{String::view_of(type)};
        const std::string name_key{String::view_of(name)};
        const auto group = m_entries.find(type_key);
        if (group != m_entries.end() && group->second.contains(name_key)) {
            m_defaults.insert_or_assign(type_key, name_key);
        }
    }

private:
    std::map<std::string, std::map<std::string, void*>> m_entries;
    std::map<std::string, std::string> m_defaults;
};

} // namespace aten_xpu

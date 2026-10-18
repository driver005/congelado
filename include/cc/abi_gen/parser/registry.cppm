export module cc_abi_gen_parser:registry;

import std;
import :vtable_model;
import :value_model;

export namespace cc_abi_gen::parser {

class Registry
{
public:
    using Iterator = std::unordered_map<std::string, vtable::Model>::iterator;
    using ConstIterator = std::unordered_map<std::string, vtable::Model>::const_iterator;

    Registry() = default;

    ~Registry() = default;
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    Registry(Registry&&) = default;
    Registry& operator=(Registry&&) = default;

    Registry& add_model(vtable::Model&& model)
    {
        m_known_models.insert({model.get_struct_name(), std::move(model)});
        return *this;
    }

    std::optional<std::reference_wrapper<vtable::Model>> find(const std::string& name)
    {
        auto it = m_known_models.find(name);
        if (it != m_known_models.end()) {
            return std::ref(it->second);
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const vtable::Model>> find(const std::string& name) const
    {
        auto it = m_known_models.find(name);
        if (it != m_known_models.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    Iterator begin() noexcept
    {
        return m_known_models.begin();
    }

    Iterator end() noexcept
    {
        return m_known_models.end();
    }

    ConstIterator begin() const noexcept
    {
        return m_known_models.begin();
    }

    ConstIterator end() const noexcept
    {
        return m_known_models.end();
    }

    void append_model(vtable::Model&& model) noexcept
    {
        m_known_models.insert({model.get_struct_name(), std::move(model)});
    }

    void append_value_model(value::Model&& model) noexcept
    {
        m_value_models.insert({model.get_c_name(), std::move(model)});
    }

    // Plain C structs and enums, keyed by their C name.
    const std::unordered_map<std::string, value::Model>& get_value_models() const noexcept
    {
        return m_value_models;
    }

    std::optional<std::reference_wrapper<const value::Model>>
    find_value(const std::string& name) const
    {
        auto it = m_value_models.find(name);
        if (it != m_value_models.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    const std::unordered_map<std::string, vtable::Model>& get_models() const noexcept
    {
        return m_known_models;
    }


private:
    std::unordered_map<std::string, vtable::Model> m_known_models;
    std::unordered_map<std::string, value::Model> m_value_models;
};

} // namespace cc_abi_gen::parser

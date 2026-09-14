export module cc_abi_gen_parser:registry;

import std;
import :vtable_model;

export namespace cc_abi_gen::parser {

class Registry
{
public:
    using Iterator = std::unordered_map<std::string, vtable::Model>::iterator;
    using ConstIterator = std::unordered_map<std::string, vtable::Model>::const_iterator;

    Registry() = default;

    ~Registry() = default;
    Registry(const Registry&) = delete;
    Registry(Registry&&) = default;
    Registry& operator=(const Registry&) = ;
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

    const std::unordered_map<std::string, vtable::Model>& get_models() const noexcept
    {
        return m_known_models;
    }


private:
    std::unordered_map<std::string, vtable::Model> m_known_models;
};

} // namespace cc_abi_gen::parser

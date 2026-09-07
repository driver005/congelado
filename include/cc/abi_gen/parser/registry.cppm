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

    void add_model(const vtable::Model&& model)
    {
        m_known_models.insert({model.get_struct_name(), std::move(model)});
    }

    std::optional<std::reference_wrapper<vtable::Model>> find(std::string_view name)
    {
        Iterator it = m_known_models.find(name);
        if (it != m_known_models.end()) {
            return std::ref(it->second);
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const vtable::Model>> find(std::string_view name) const
    {
        ConstIterator it = m_known_models.find(name);
        if (it != m_known_models.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    Iterator begin()
    {
        return m_known_models.begin();
    }

    Iterator end()
    {
        return m_known_models.end();
    }

    ConstIterator begin() const
    {
        return m_known_models.begin();
    }

    ConstIterator end() const
    {
        return m_known_models.end();
    }

private:
    std::unordered_map<std::string, vtable::Model> m_known_models;
};

} // namespace cc_abi_gen::parser

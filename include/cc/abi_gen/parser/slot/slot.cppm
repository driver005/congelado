module;
#include <functional>
export module cc_abi_gen_parser:slot_slot;

import std;
import :helper_parameter;

export namespace cc_abi_gen::parser::slot {

class Slot
{
public:
    Slot(
        std::string&& name,
        std::string&& return_type,
        std::vector<helper::Parameter>&& parameters
    ) :
        m_name{std::move(name)},
        m_return_type{std::move(return_type)},
        m_parameters{std::move(parameters)}
    {
    }

    std::span<const helper::Parameter> extract_parameters()
    {
        if (has_parameter()) {
            auto params = std::span<const helper::Parameter>{m_parameters}.subspan(1);

            if (is_failable() && !params.empty()) {
                return params.first(params.size() - 1);
            }

            return params;
        }

        return {};
    }

    std::optional<std::reference_wrapper<helper::Parameter>> extract_failable()
    {
        if (has_parameter() && is_failable()) {
            return std::ref(m_parameters.back());
        }

        return std::nullopt;
    }

    bool is_failable()
    {
        return has_parameter() && m_parameters.back().get_pointee_name() == "TF_Status";
    }

    bool is_destroy()
    {
        return m_name == "destroy";
    }

    bool is_get_name()
    {
        return m_name == "get_name";
    }

    bool has_parameter()
    {
        return !m_parameters.empty();
    }

    // Example: Create
    const std::string& get_name() const
    {
        return m_name;
    }

    // Example: TF_TString
    const std::string& get_return_type() const
    {
        return m_return_type;
    }

    const std::span<const helper::Parameter> get_parameters() const
    {
        return std::span<const helper::Parameter>{m_parameters};
    }

private:
    std::string m_name;
    std::string m_return_type;
    std::vector<helper::Parameter> m_parameters;
};

} // namespace cc_abi_gen::parser::slot

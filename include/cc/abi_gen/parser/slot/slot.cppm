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

    ~Slot() = default;
    Slot(const Slot&) = delete;
    Slot& operator=(const Slot&) = delete;
    Slot(Slot&&) = default;
    Slot& operator=(Slot&&) = default;

    Slot& add_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
        return *this;
    }

    Slot& add_return_type(std::string&& return_type) noexcept
    {
        m_return_type = std::move(return_type);
        return *this;
    }

    Slot& add_parameter(helper::Parameter&& parameter) noexcept
    {
        m_parameters.emplace_back(std::move(parameter));
        return *this;
    }

    std::span<const helper::Parameter> extract_parameters() const noexcept
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

    std::optional<std::reference_wrapper<const helper::Parameter>> extract_failable() const noexcept
    {
        if (has_parameter() && is_failable()) {
            return std::ref(m_parameters.back());
        }

        return std::nullopt;
    }

    bool is_failable() const
    {
        return has_parameter() && m_parameters.back().get_pointee_name() == "TF_Status";
    }

    bool is_destroy() const
    {
        return m_name == "destroy";
    }

    bool is_get_name() const
    {
        return m_name == "get_name";
    }

    bool has_parameter() const
    {
        return !m_parameters.empty();
    }

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    void set_retrun_type(std::string&& return_type) noexcept
    {
        m_return_type = std::move(return_type);
    }

    void append_parameter(helper::Parameter&& parameter) noexcept
    {
        m_parameters.emplace_back(std::move(parameter));
    }

    // Example: Create
    const std::string& get_name() const noexcept
    {
        return m_name;
    }

    // Example: TF_TString
    const std::string& get_return_type() const noexcept
    {
        return m_return_type;
    }

     std::span<const helper::Parameter> get_parameters() const noexcept
    {
        return std::span<const helper::Parameter>{m_parameters};
    }

private:
    std::string m_name;
    std::string m_return_type;
    std::vector<helper::Parameter> m_parameters;
};

} // namespace cc_abi_gen::parser::slot

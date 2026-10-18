module;

#include <cctype>

export module cc_abi_gen_parser:value_model;

import std;
import :value_field;

export namespace cc_abi_gen::parser::value {

enum class Kind
{
    Struct,
    Enum
};

// The parsed shape of a plain C struct or enum that is not part of a vtable flow.
class Model
{
public:
    Model(
        Kind kind,
        std::string&& c_name,
        std::string&& class_name,
        std::string&& domain_name,
        std::string&& header_path
    ) :
        m_kind(kind),
        m_c_name(std::move(c_name)),
        m_class_name(std::move(class_name)),
        m_domain_name(std::move(domain_name)),
        m_header_path(std::move(header_path))
    {
    }

    ~Model() = default;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    Model& add_field(Field&& field) noexcept
    {
        m_fields.push_back(std::move(field));
        return *this;
    }

    Model& add_enumerator(std::string&& enumerator) noexcept
    {
        m_enumerators.push_back(std::move(enumerator));
        return *this;
    }

    Model& add_struct_size(bool has_struct_size) noexcept
    {
        m_struct_size = has_struct_size;
        return *this;
    }

    Kind get_kind() const noexcept
    {
        return m_kind;
    }

    // Example: TF_PoolId
    const std::string& get_c_name() const noexcept
    {
        return m_c_name;
    }

    // Example: PoolId
    const std::string& get_class_name() const noexcept
    {
        return m_class_name;
    }

    // Example: stream_executor
    const std::string& get_domain_name() const noexcept
    {
        return m_domain_name;
    }

    // Example: include/c/extern/stream_executor/types.h
    const std::string& get_header_path() const noexcept
    {
        return m_header_path;
    }

    bool has_struct_size() const noexcept
    {
        return m_struct_size;
    }

    std::span<const Field> get_fields() const noexcept
    {
        return m_fields;
    }

    // C enumerator names in declaration order.
    std::span<const std::string> get_enumerators() const noexcept
    {
        return m_enumerators;
    }

    // Partition and file stem. Example: PoolId -> pool_id
    std::string to_file_name() const
    {
        std::string name;
        for (const char character: m_class_name) {
            if (std::isupper(static_cast<unsigned char>(character)) && !name.empty()) {
                name += '_';
            }
            name += static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }

        return name;
    }

    // Tier folder of the header. Example: include/c/extern/stream_executor/types.h -> include/c/extern
    std::string to_tier() const
    {
        const std::filesystem::path path{m_header_path};

        return path.parent_path().parent_path().generic_string();
    }

private:
    Kind m_kind;
    std::string m_c_name;
    std::string m_class_name;
    std::string m_domain_name;
    std::string m_header_path;
    bool m_struct_size{false};
    std::vector<Field> m_fields;
    std::vector<std::string> m_enumerators;
};

} // namespace cc_abi_gen::parser::value

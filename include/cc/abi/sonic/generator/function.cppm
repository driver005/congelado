module;

#include "c/extern/generator/generator.h"

export module cc_abi_sonic_generator:function;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
export namespace ice::sonic {

// C ABI adapter: implements ice::builder::Function by calling
// TF_Generator_Function_* functions. Owning (unlike the non-owning Definition/Parameter/
// Attribute adapters in this module — this wraps a real opened construction resource) — same
// ownership shape as Runtime itself owning its TF_Generator_Controller*: non-copyable, destroys
// the C handle in its destructor.
class Function : public ice::builder::Function
{
public:
    explicit Function(TF_Generator* ops, void* handle) :
        m_ops{ops}, m_handle{handle}
    {
    }

    ~Function()
    {

        if (m_handle && m_ops) {
            m_ops->function__destroy(m_handle);
        }
    }

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
    Function(Function&&) = delete;
    Function& operator=(Function&&) = delete;

    std::expected<ice::builder::NodeHandle, ice::Status>
    add_parameter(std::string_view name, std::string_view type_text)
    {

        ice::Status status;
        std::size_t id = 0;
        bool ok = m_ops->function__add_parameter(
            m_handle, *ice::String(std::string{name}).get_handle(), *ice::String(std::string{type_text}).get_handle(), &id,
            status.get_handle()
        );
        if (!ok) {
            return std::unexpected{status};
        }
        return ice::builder::NodeHandle{id};
    }

    std::expected<void, ice::Status> add_node(
        std::string_view kind,
        std::span<const ice::builder::NodeHandle> operands,
        std::span<const std::pair<std::string_view, std::string_view>> attrs,
        std::span<ice::builder::NodeHandle> out_results
    )
    {

        static_assert(sizeof(ice::builder::NodeHandle) == sizeof(std::size_t));
        static_assert(alignof(ice::builder::NodeHandle) == alignof(std::size_t));
        const std::size_t* operand_ids = std::bit_cast<const std::size_t*>(operands.data());
        std::size_t* out_result_ids = std::bit_cast<std::size_t*>(out_results.data());

        // attrs need null-terminated C strings for the const char* C ABI — string_view isn't
        // null-terminated, so this temporary is inherent to TF_Generator_Function_AddNode's
        // signature (same class of unavoidable temporary as add_parameter's local strings above).
        std::vector<ice::String> attr_keys_sr;
        std::vector<ice::String> attr_values_sr;
        std::vector<TF_String> attr_keys;
        std::vector<TF_String> attr_values;
        attr_keys_sr.reserve(attrs.size());
        attr_values_sr.reserve(attrs.size());
        attr_keys.reserve(attrs.size());
        attr_values.reserve(attrs.size());
        for (const auto& [key, value]: attrs) {
            attr_keys_sr.emplace_back(std::string{key});
            attr_values_sr.emplace_back(std::string{value});
            attr_keys.push_back(*attr_keys_sr.back().get_handle());
            attr_values.push_back(*attr_values_sr.back().get_handle());
        }

        ice::Status status;
        m_ops->function__add_node(
            m_handle, *ice::String(std::string{kind}).get_handle(), operand_ids, operands.size(), attr_keys.data(),
            attr_values.data(), attr_keys.size(), out_results.size(), out_result_ids,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status>
    exit_border_patrol(std::span<const ice::builder::NodeHandle> outputs)
    {

        static_assert(sizeof(ice::builder::NodeHandle) == sizeof(std::size_t));
        static_assert(alignof(ice::builder::NodeHandle) == alignof(std::size_t));
        const std::size_t* output_ids = std::bit_cast<const std::size_t*>(outputs.data());
        ice::Status status;
        bool ok = m_ops->function__exit_border_patrol(
            m_handle, output_ids, outputs.size(), status.get_handle()
        );
        if (!ok) {
            return std::unexpected{status};
        }
        return {};
    }

    TF_Generator* m_ops;
    void* m_handle; // owning — this object destroys it
};

} // namespace ice::sonic

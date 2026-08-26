module;

#include "c/extern/generator/controller.h"

export module cc_abi_sonic_generator:function;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;

export namespace ice::sonic::generator {

// C ABI adapter: implements ice::builder::generator::Function by calling
// TF_Generator_Function_* functions. Owning (unlike the non-owning Definition/Parameter/
// Attribute adapters in this module — this wraps a real opened construction resource) — same
// ownership shape as Runtime itself owning its TF_Generator_Controller*: non-copyable, destroys
// the C handle in its destructor.
class Function : public ice::builder::generator::Function
{
public:
    explicit Function(TF_Generator_Function* handle) :
        m_handle{handle}
    {
    }

    ~Function() override
    {

        if (m_handle) {
            TF_Generator_Function_Destroy(m_handle);
        }
    }

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
    Function(Function&&) = delete;
    Function& operator=(Function&&) = delete;

    std::expected<ice::builder::generator::NodeHandle, ice::sonic::StringRuntime>
    add_parameter(std::string_view name, std::string_view type_text) override
    {

        TF_Status* status = TF_NewStatus();
        std::size_t id = 0;
        bool ok = TF_Generator_Function_AddParameter(
            m_handle, std::string{name}.c_str(), std::string{type_text}.c_str(), &id, status
        );
        if (!ok) {
            ice::sonic::StringRuntime message{std::string{TF_Message(status)}};
            TF_DeleteStatus(status);
            return std::unexpected{message};
        }
        TF_DeleteStatus(status);
        return ice::builder::generator::NodeHandle{id};
    }

    bool add_node(
        std::string_view kind,
        std::span<const ice::builder::generator::NodeHandle> operands,
        std::span<const std::pair<std::string_view, std::string_view>> attrs,
        std::span<ice::builder::generator::NodeHandle> out_results
    ) override
    {

        static_assert(sizeof(ice::builder::generator::NodeHandle) == sizeof(std::size_t));
        static_assert(alignof(ice::builder::generator::NodeHandle) == alignof(std::size_t));
        const std::size_t* operand_ids = reinterpret_cast<const std::size_t*>(operands.data());
        std::size_t* out_result_ids = reinterpret_cast<std::size_t*>(out_results.data());

        // attrs need null-terminated C strings for the const char* C ABI — string_view isn't
        // null-terminated, so this temporary is inherent to TF_Generator_Function_AddNode's
        // signature (same class of unavoidable temporary as add_parameter's local strings above).
        std::vector<std::string> attr_key_storage;
        std::vector<std::string> attr_value_storage;
        std::vector<const char*> attr_keys;
        std::vector<const char*> attr_values;
        attr_key_storage.reserve(attrs.size());
        attr_value_storage.reserve(attrs.size());
        attr_keys.reserve(attrs.size());
        attr_values.reserve(attrs.size());
        for (const auto& [key, value]: attrs) {
            attr_key_storage.emplace_back(key);
            attr_value_storage.emplace_back(value);
            attr_keys.push_back(attr_key_storage.back().c_str());
            attr_values.push_back(attr_value_storage.back().c_str());
        }

        TF_Status* status = TF_NewStatus();
        TF_Generator_Function_AddNode(
            m_handle, std::string{kind}.c_str(), operand_ids, operands.size(), attr_keys.data(),
            attr_values.data(), attr_keys.size(), out_results.size(), out_result_ids, status
        );
        bool ok = TF_GetCode(status) == TF_OK;
        TF_DeleteStatus(status);
        return ok;
    }

    bool exit_border_patrol(std::span<const ice::builder::generator::NodeHandle> outputs) override
    {

        static_assert(sizeof(ice::builder::generator::NodeHandle) == sizeof(std::size_t));
        static_assert(alignof(ice::builder::generator::NodeHandle) == alignof(std::size_t));
        const std::size_t* output_ids = reinterpret_cast<const std::size_t*>(outputs.data());
        TF_Status* status = TF_NewStatus();
        bool ok = TF_Generator_Function_ExitBorderPatrol(m_handle, output_ids, outputs.size(), status);
        TF_DeleteStatus(status);
        return ok;
    }

private:
    TF_Generator_Function* m_handle; // owning — this object destroys it
};

} // namespace ice::sonic::generator

module;

#include "c/extern/generator/attribute.h"
#include "c/extern/generator/definition.h"
#include "c/extern/generator/parameter.h"
#include "c/extern/generator/typeinfo.h"

export module cc_abi_sonic_generator:definition;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;
import :parameter;
import :attribute;

export namespace ice::sonic::generator {

// C ABI adapter: implements ice::builder::generator::Definition by calling
// TF_Generator_Definition_* functions.
class Definition : public ice::builder::generator::Definition
{
public:
    explicit Definition(const TF_Generator_Definition* handle) :
        m_handle(handle)
    {
    }

    ~Definition() override = default;

    Definition(const Definition&) = default;
    Definition& operator=(const Definition&) = default;
    Definition(Definition&&) = default;
    Definition& operator=(Definition&&) = default;

    StringRuntime get_name() const override
    {

        return StringRuntime(TF_Generator_Definition_GetName(m_handle));
    }

    StringRuntime get_summary() const override
    {

        return StringRuntime(TF_Generator_Definition_GetSummary(m_handle));
    }

    StringRuntime get_description() const override
    {

        return StringRuntime(TF_Generator_Definition_GetDescription(m_handle));
    }

    size_t get_input_count() const override
    {

        return TF_Generator_Definition_GetInputCount(m_handle);
    }

    std::unique_ptr<ice::builder::generator::Parameter> get_input(size_t index) const override
    {

        const TF_Generator_Parameter* param = TF_Generator_Definition_GetInput(m_handle, index);
        if (!param) {
            return nullptr;
        }
        return std::make_unique<Parameter>(param);
    }

    size_t get_output_count() const override
    {

        return TF_Generator_Definition_GetOutputCount(m_handle);
    }

    std::unique_ptr<ice::builder::generator::Parameter> get_output(size_t index) const override
    {

        const TF_Generator_Parameter* param = TF_Generator_Definition_GetOutput(m_handle, index);
        if (!param) {
            return nullptr;
        }
        return std::make_unique<Parameter>(param);
    }

    size_t get_attr_count() const override
    {

        return TF_Generator_Definition_GetAttrCount(m_handle);
    }

    std::unique_ptr<ice::builder::generator::Attribute> get_attr(size_t index) const override
    {

        const TF_Generator_Attribute* attr = TF_Generator_Definition_GetAttr(m_handle, index);
        if (!attr) {
            return nullptr;
        }
        return std::make_unique<Attribute>(attr);
    }

    const TF_Generator_Definition* get_handle() const
    {
        return m_handle;
    }

private:
    const TF_Generator_Definition* m_handle;
};

} // namespace ice::sonic::generator

module;

#include "c/extern/generator/parameter.h"
#include "c/extern/generator/typeinfo.h"

export module cc_abi_sonic_generator:parameter;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;
import :typeinfo;

export namespace ice::sonic::generator {

// C ABI adapter: implements ice::builder::generator::Parameter by calling
// TF_Generator_Parameter_* functions.
class Parameter : public ice::builder::generator::Parameter
{
public:
    explicit Parameter(const TF_Generator_Parameter* handle) :
        m_handle(handle)
    {
    }

    ~Parameter() override = default;

    Parameter(const Parameter&) = default;
    Parameter& operator=(const Parameter&) = default;
    Parameter(Parameter&&) = default;
    Parameter& operator=(Parameter&&) = default;

    StringRuntime get_name() const override
    {

        return StringRuntime(TF_Generator_Parameter_GetName(m_handle));
    }

    StringRuntime get_description() const override
    {

        return StringRuntime(TF_Generator_Parameter_GetDescription(m_handle));
    }

    int get_position() const override
    {

        return TF_Generator_Parameter_GetPosition(m_handle);
    }

    std::unique_ptr<ice::builder::generator::TypeInfo> get_type() const override
    {

        const TF_Generator_TypeInfo* type = TF_Generator_Parameter_GetType(m_handle);
        if (!type) {
            return nullptr;
        }
        return std::make_unique<TypeInfo>(type);
    }

    const TF_Generator_Parameter* get_handle() const
    {
        return m_handle;
    }

private:
    const TF_Generator_Parameter* m_handle;
};

} // namespace ice::sonic::generator

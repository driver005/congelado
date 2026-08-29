module;

export module cc_abi_builder_generator:function;

import std;
import cc_abi_primitives;
#include "c/intern/tf_tensor.h"
import cc_abi_sonic_intern;
import :parameter;
import :definition;
import :attribute;

export namespace ice::builder {

class Function
{
public:
    virtual ~Function() = default;

    virtual std::expected<std::unique_ptr<Parameter>, ice::Status>
    add_parameter(const ice::String& name, const ice::String& type_text) = 0;

    virtual std::expected<void, ice::Status> add_node(
        const Definition& def,
        ice::TensorHandle operands,
        ice::TensorHandle attrs,
        ice::TensorHandle out_results
    ) = 0;

    virtual std::expected<void, ice::Status>
    exit_border_patrol(ice::TensorHandle outputs) = 0;
};

} // namespace ice::builder

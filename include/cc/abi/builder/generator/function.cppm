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
    // Recover the Function instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Function* create(void* ctx) noexcept
    {
        return static_cast<Function*>(ctx);
    }

    virtual ~Function() = default;

    virtual [[nodiscard]] std::expected<std::unique_ptr<Parameter>, ice::Status>
    add_parameter(const ice::String& name, const ice::String& type_text) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status> add_node(
        const Definition& def,
        ice::TensorHandle operands,
        ice::TensorHandle attrs,
        ice::TensorHandle out_results
    ) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    exit_border_patrol(ice::TensorHandle outputs) = 0;
};

} // namespace ice::builder

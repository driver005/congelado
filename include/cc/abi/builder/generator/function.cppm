module;

#include "c/intern/tf_tensor.h"

export module cc_abi_builder_generator:function;

import std;
import cc_abi_primitives;
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

    [[nodiscard]] virtual std::expected<std::unique_ptr<Parameter>, ice::Status>
    add_parameter(const ice::String& name, const ice::String& type_text) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> add_node(
        const Definition& def,
        ice::TensorHandle operands,
        ice::TensorHandle attrs,
        ice::TensorHandle out_results
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    exit_border_patrol(ice::TensorHandle outputs) noexcept = 0;
};

} // namespace ice::builder

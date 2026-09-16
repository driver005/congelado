// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/attrtype/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/attrtype/attrtype.h"

export module cc_abi_sonic_attrtype;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Attrtype : public ice::sonic::Runtime<Attrtype, TF_AttrType>
{
public:
    explicit Attrtype(TF_AttrType* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "attrtype";

    [[nodiscard]] std::expected<void, ice::Status>
    attrtype_name(TF_AttrType_Enum type, TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->attrtype_name(get_handle(), type, out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic

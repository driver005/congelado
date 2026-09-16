// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/datatype/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/datatype/datatype.h"

export module cc_abi_sonic_datatype;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Datatype : public ice::sonic::Runtime<Datatype, TF_DataType>
{
public:
    explicit Datatype(TF_DataType* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "datatype";

    [[nodiscard]] std::expected<void, ice::Status> datatype_size(TF_DataType_Enum dt) noexcept
    {
        ice::Status status;
        m_ops->datatype_size(get_handle(), dt, status.get_handle());

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

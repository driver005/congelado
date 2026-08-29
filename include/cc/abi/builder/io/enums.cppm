module;

#include "c/extern/io/enums.h"

export module cc_abi_builder_io:enums;

export namespace ice::builder::io {

enum class Method
{
    Get = TF_IO_GET,
    Post = TF_IO_POST,
    Put = TF_IO_PUT,
    Delete = TF_IO_DELETE,
    Patch = TF_IO_PATCH,
    Head = TF_IO_HEAD,
    Options = TF_IO_OPTIONS,
    Connect = TF_IO_CONNECT,
    Trace = TF_IO_TRACE,
};

inline TF_IO_Method method_to_c(Method m) noexcept {
    return static_cast<TF_IO_Method>(m);
}
inline Method method_from_c(TF_IO_Method m) noexcept {
    return static_cast<Method>(m);
}

} // namespace ice::builder

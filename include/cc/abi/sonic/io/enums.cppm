module;

#include "c/extern/io/enums.h"

export module cc_abi_sonic_io:enums;

export namespace ice::sonic::io {

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

} // namespace ice::sonic

#ifndef TENSORFLOW_C_EXTERN_CLIENT_CLIENT_H_
#define TENSORFLOW_C_EXTERN_CLIENT_CLIENT_H_

// abi_gen derives a struct's domain purely from its own name (TF_Client -> "client") and
// module_header.inja hardcodes #include "c/extern/<domain>/<domain>.h" from that. The real
// header lives under io/ (io.h aggregates client.h/request.h/response.h/server.h/socket.h),
// so this redirect makes the generated path resolve.
#include "c/extern/io/client.h"

#endif

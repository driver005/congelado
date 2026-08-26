module;

#include <cassert>

export module cc_tmp:ops_op_requires;

import std;
import cc_abi;

export {

    namespace tensorflow {

#define OP_REQUIRES(CTX, EXP, STATUS)                                                              \
    do {                                                                                           \
        if (!(EXP)) {                                                                              \
            (CTX)->Failure(STATUS.get_code());                                                     \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define OP_REQUIRES_OK(CTX, STATUS)                                                                \
    do {                                                                                           \
        if (!(STATUS).ok()) {                                                                      \
            (CTX)->Failure(STATUS.get_code());                                                     \
            return;                                                                                \
        }                                                                                          \
    } while (0)

    } // namespace tensorflow

} // export

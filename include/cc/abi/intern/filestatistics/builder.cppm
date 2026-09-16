// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/filestatistics/filestatistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/filestatistics/filestatistics.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_filestatistics;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Filestatistics
{
public:
    static Filestatistics* create(void* ctx) noexcept
    {
        return static_cast<Filestatistics*>(ctx);
    }

    template<typename HandleT>
    static Filestatistics* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Filestatistics*>(handle);
    }

    virtual ~Filestatistics() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_FileStatistics* get_generic_vtable()
    {
        static TF_FileStatistics vtable = {
            .struct_size = TF_FILESTATISTICS_STRUCT_SIZE,

        };

        return &vtable;
    }
};

} // namespace ice::builder

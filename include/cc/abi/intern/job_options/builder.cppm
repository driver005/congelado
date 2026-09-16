// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/job_options/job_options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/job_options/job_options.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_job_options;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Job_options
{
public:
    static Job_options* create(void* ctx) noexcept
    {
        return static_cast<Job_options*>(ctx);
    }

    template<typename HandleT>
    static Job_options* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Job_options*>(handle);
    }

    virtual ~Job_options() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Job_Options* get_generic_vtable()
    {
        static TF_Job_Options vtable = {
            .struct_size = TF_JOB_OPTIONS_STRUCT_SIZE,

        };

        return &vtable;
    }
};

} // namespace ice::builder

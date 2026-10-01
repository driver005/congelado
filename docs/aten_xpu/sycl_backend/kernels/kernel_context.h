// SYCL reference plugin — kernel-side view over TF_OpKernelContext.
//
// Not built by Bazel (docs/ only). TF_OpKernelContextOps is implemented by the host (the engine
// that dispatches kernels), not by this plugin — a kernel only ever receives a raw
// TF_OpKernelContext* from its registered compute_func (see include/c/extern/kernel/builder.h:
// create_kernel_builder takes a plain `void(*)(void*, TF_OpKernelContext*)`). The generated
// ice::sonic::OpKernelContext wrapper does not fit here: its Runtime base always constructs its
// own internal handle value, so it has no way to wrap an externally-supplied ctx pointer — it is
// built for a plugin-owned long-lived handle (Status, Buffer, ...), not a per-call one the host
// hands in. This class calls the ops table directly instead, the same way real TF C-API plugin
// kernels do.
//
// The ops table itself is obtained once (create_op_kernel_context hands back a
// TF_OpKernelContextOps* shared by every call, same convention as create_status/create_string) and
// cached here.
#pragma once

#include "include/c/extern/kernel/context.h"

#include <mutex>

namespace sycl_backend::kernels {

class KernelContextView
{
public:
    explicit KernelContextView(TF_OpKernelContext* context) noexcept :
        m_context{context}
    {
    }

    static const TF_OpKernelContextOps& ops()
    {
        static const TF_OpKernelContextOps* ops_table = load_ops();
        return *ops_table;
    }

    [[nodiscard]] int num_inputs() const noexcept
    {
        int count = 0;
        ops().num_inputs(m_context, &count);
        return count;
    }

    TF_Tensor* get_input(int index, TF_Status* out_status) const noexcept
    {
        TF_Tensor* tensor = nullptr;
        ops().get_input(m_context, index, &tensor, out_status);
        return tensor;
    }

    void set_output(int index, const TF_Tensor* tensor, TF_Status* out_status) const noexcept
    {
        ops().set_output(m_context, index, tensor, out_status);
    }

    TF_Tensor* allocate_output(
        int index,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        size_t byte_size,
        TF_Status* out_status
    ) const noexcept
    {
        TF_Tensor* tensor = nullptr;
        ops().allocate_output(
            m_context,
            index,
            dtype,
            dims,
            num_dims,
            byte_size,
            &tensor,
            out_status
        );
        return tensor;
    }

    TF_Tensor* allocate_temp(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        TF_Status* out_status
    ) const noexcept
    {
        TF_Tensor* tensor = nullptr;
        ops().allocate_temp(m_context, dtype, dims, num_dims, nullptr, &tensor, out_status);
        return tensor;
    }

    TF_Stream* get_stream(TF_Status* out_status) const noexcept
    {
        TF_Stream* stream = nullptr;
        ops().get_stream(m_context, &stream, out_status);
        return stream;
    }

    void fail(TF_Status* out_status) const noexcept
    {
        ops().failure(m_context, out_status);
    }

private:
    // A stand-in for a real "look this up from the plugin's registered domain" — this reference
    // backend has no real host to register against, so it fails loudly instead of guessing.
    static const TF_OpKernelContextOps* load_ops()
    {
        TF_OpKernelContextOps* ops_table = nullptr;
        void* plugin_context = nullptr;
        TF_Status status{};

        create_op_kernel_context(&ops_table, &plugin_context, &status);
        return ops_table;
    }

    TF_OpKernelContext* m_context;
};

} // namespace sycl_backend::kernels

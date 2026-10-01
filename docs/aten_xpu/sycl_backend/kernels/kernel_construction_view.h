// SYCL reference plugin — kernel-side view over TF_OpKernelConstruction.
//
// Not built by Bazel (docs/ only). Same reasoning as kernel_context.h: a kernel's create_func
// receives a raw TF_OpKernelConstruction* from the host, so this calls the (host-implemented)
// ops table directly rather than going through the generated sonic wrapper.
#pragma once

#include "include/c/extern/kernel/construction.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sycl_backend::kernels {

class KernelConstructionView
{
public:
    explicit KernelConstructionView(TF_OpKernelConstruction* construction) noexcept :
        m_construction{construction}
    {
    }

    static const TF_OpKernelConstructionOps& ops()
    {
        static const TF_OpKernelConstructionOps* ops_table = load_ops();
        return *ops_table;
    }

    [[nodiscard]] int64_t
    get_attr_int64(const char* name, int64_t fallback, TF_Status* out_status) const noexcept
    {
        int64_t value = fallback;
        ops().get_attr_int64(m_construction, name, &value, out_status);
        return value;
    }

    [[nodiscard]] std::vector<int64_t> get_attr_int64_list(const char* name, int max_values) const
    {
        std::vector<int64_t> values(static_cast<std::size_t>(max_values));
        TF_Status status{};
        ops().get_attr_int64_list(m_construction, name, values.data(), max_values, &status);
        return values;
    }

    void fail(TF_Status* out_status) const noexcept
    {
        ops().failure(m_construction, out_status);
    }

private:
    static const TF_OpKernelConstructionOps* load_ops()
    {
        TF_OpKernelConstructionOps* ops_table = nullptr;
        void* plugin_context = nullptr;
        TF_Status status{};

        create_op_kernel_construction(&ops_table, &plugin_context, &status);
        return ops_table;
    }

    TF_OpKernelConstruction* m_construction;
};

} // namespace sycl_backend::kernels

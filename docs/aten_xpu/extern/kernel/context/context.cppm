module;

#include "include/c/extern/kernel/context.h"
#include "include/c/extern/stream_executor/types.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:context;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_kernel_sonic;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;

export namespace aten_xpu {

class SyclKernelContext
{
public:
    explicit SyclKernelContext(::TF_OpKernelContext* context) :
        m_ops{SyclOpsTable::getInstance()},
        m_helper{m_ops},
        m_context{m_ops.getKernelContextOps(), context},
        m_status{m_ops.getStatusOps()}
    {
        m_status.create();
    }

    ~SyclKernelContext()
    {
        m_status.destroy();
    }

    SyclKernelContext(const SyclKernelContext&) = delete;
    SyclKernelContext& operator=(const SyclKernelContext&) = delete;
    SyclKernelContext(SyclKernelContext&&) = delete;
    SyclKernelContext& operator=(SyclKernelContext&&) = delete;

    int getInputCount() const noexcept
    {
        int count = 0;
        m_context.num_inputs(&count);
        return count;
    }

    std::optional<std::reference_wrapper<SyclTensor>> getInput(int index)
    {
        if (index >= getInputCount()) {
            return std::nullopt;
        }
        ::TF_Tensor* handle = nullptr;
        m_context.get_input(index, &handle, m_status);
        if (!ok() || handle == nullptr) {
            return std::nullopt;
        }
        return SyclHandle::resolve_raw<SyclTensor>(handle);
    }

    std::optional<std::reference_wrapper<SyclTensor>>
    allocateOutput(int index, TFDataTypeEnum dtype, std::span<const int64_t> dims)
    {
        ::TF_Tensor* handle = nullptr;
        const auto elements =
            std::accumulate(dims.begin(), dims.end(), int64_t{1}, std::multiplies<>{});
        const auto bytes = static_cast<std::size_t>(elements) * SyclTensor::element_size(dtype);
        m_context.allocate_output(
            index,
            dtype,
            dims.data(),
            static_cast<int>(dims.size()),
            bytes,
            &handle,
            m_status
        );
        if (!ok() || handle == nullptr) {
            return std::nullopt;
        }
        return SyclHandle::resolve_raw<SyclTensor>(handle);
    }

    std::optional<std::reference_wrapper<SyclTensor>>
    allocateTemp(TFDataTypeEnum dtype, std::span<const int64_t> dims)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.allocate_temp(
            dtype,
            dims.data(),
            static_cast<int>(dims.size()),
            nullptr,
            &handle,
            m_status
        );
        if (!ok() || handle == nullptr) {
            return std::nullopt;
        }
        return SyclHandle::resolve_raw<SyclTensor>(handle);
    }

    std::optional<std::reference_wrapper<sycl::queue>> getQueue()
    {
        ::TF_Stream* stream = nullptr;
        m_context.get_stream(&stream, m_status);
        if (!ok() || stream == nullptr) {
            return std::nullopt;
        }
        return SyclHandle::resolve_raw<SyclStream>(stream).getNativeQueue();
    }

    int getDeviceId() const noexcept
    {
        int device = 0;
        m_context.get_device_id(&device);
        return device;
    }

    const ice::sonic::Status& getStatus() const noexcept
    {
        return m_status;
    }

    bool ok() const noexcept
    {
        TF_Code code = TF_OK;
        m_status.get_code(&code);
        return code == TF_OK;
    }

    void fail(TF_Code code, std::string_view message)
    {
        m_helper.fail(m_status, code, message);
        m_context.failure(m_status);
    }

    void fail_from(const std::exception& error)
    {
        fail(TF_INTERNAL, error.what());
    }

    void propagate()
    {
        if (!ok()) {
            m_context.failure(m_status);
        }
    }

private:
    const SyclOpsTable& m_ops;
    SyclStatus m_helper;
    ice::sonic::TF_OpKernelContextOps m_context;
    ice::sonic::Status m_status;
};

} // namespace aten_xpu

module;

#include "include/c/extern/stream_executor/stream.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_grappler:capture_status;

import std;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;

export namespace aten_xpu {

class SyclCaptureStatus
{
public:
    SyclCaptureStatus() = delete;

    static bool is_recording(const sycl::queue& queue)
    {
        return queue.ext_oneapi_get_state() ==
               sycl::ext::oneapi::experimental::queue_state::recording;
    }

    static void assert_not_capturing(const sycl::queue& queue, std::string_view attempt)
    {
        if (is_recording(queue)) {
            throw std::logic_error{std::format("{} during XPU graph capture", attempt)};
        }
    }

    static void filter_capture_stream(void* filter_data, ::TF_Stream* stream, _Bool* out_match)
    {
        const auto* capture_queue = static_cast<const sycl::queue*>(filter_data);
        auto& candidate = SyclHandle::resolve_raw<SyclStream>(stream).getNativeQueue();
        *out_match = is_recording(candidate) && candidate == *capture_queue;
    }
};

} // namespace aten_xpu

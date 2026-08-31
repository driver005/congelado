module;

#include "c/extern/profiler/profiler.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_profiler;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a profiler backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Generator's role. A backend implements this directly; the
// mainframe reaches it only through the C ABI (see ice::sonic::Profiler — no
// in-process fast path, so an independently-built third-party profiler plugin never needs to
// share the host's exact C++ ABI/toolchain).
class Profiler
{
public:
    // Recover the Profiler instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Profiler* create(void* ctx) noexcept
    {
        return static_cast<Profiler*>(ctx);
    }

    virtual ~Profiler() = default;

    virtual ice::String get_name() const noexcept = 0;

    virtual ice::String get_device_type() const noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop() noexcept = 0;

    // Returns a plugin-allocated 1-D Uint8 tensor of the collected xspace data;
    // ownership transfers to the caller (release with the tensor runtime's delete).
    [[nodiscard]] virtual std::expected<ice::TensorHandle, ice::Status> collect_data_xspace() noexcept = 0;

    static TF_Profiler* get_generic_vtable()
    {
        static TF_Profiler vtable = {
            .struct_size = TF_PROFILER_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Profiler::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get_device_type =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto name = self->get_device_type();
                name.to_c(out);
            },
            .start =
                [](void* plugin_context, TF_Status* status) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop =
                [](void* plugin_context, TF_Status* status) noexcept
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .collect_data_xspace =
                [](void* plugin_context, TF_Status* status) noexcept -> TF_Tensor_Handle*
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->collect_data_xspace();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->get_handle();
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder

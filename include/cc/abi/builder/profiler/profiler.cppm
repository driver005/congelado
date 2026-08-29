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
// mirrors ice::builder::Builder's role. A backend implements this directly; the
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

    virtual ice::String get_device_type() const = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status> start() = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status> stop() = 0;

    // buffer == nullptr means "just report the required size in *size_in_bytes" (matches the
    // original contract — first call with a null buffer sizes it, a second call with a
    // buffer of that size fills it).
    virtual [[nodiscard]] std::expected<void, ice::Status>
    collect_data_xspace(std::uint8_t* buffer, std::size_t* size_in_bytes) = 0;

    static TF_Profiler* get_generic_vtable()
    {
        static TF_Profiler vtable = {
            .struct_size = sizeof(TF_Profiler),
            .destroy =
                [](void* plugin_context)
            {
                delete Profiler::create(plugin_context);
            },
            .get_device_type =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Profiler::create(plugin_context);
                auto name = self->get_device_type();
                name.to_c(out);
            },
            .start =
                [](void* plugin_context, TF_Status* status)
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop =
                [](void* plugin_context, TF_Status* status)
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .collect_data_xspace =
                [](void* plugin_context, uint8_t* buffer, size_t* size_in_bytes, TF_Status* status)
            {
                auto* self = Profiler::create(plugin_context);
                auto res = self->collect_data_xspace(buffer, size_in_bytes);
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder

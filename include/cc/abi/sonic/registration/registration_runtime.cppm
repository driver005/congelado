module;

#include "c/extern/registration/registration.h"
#include "c/intern/tf_status.h"

export module cc_abi_sonic_registration:registration_runtime;

import std;
import cc_abi_primitives;

export namespace ice::sonic {

// Thin wrapper around c/extern/registration/registration.h's generic named-value registry —
// the genuinely process-wide (linkshared, see include/c/BUILD's registration_shared target)
// storage that lets a dynamically loaded plugin .so register a factory function pointer
// visible to the host process. A plain in-process singleton would break here: statically
// linking the same singleton code into both the host and a plugin .so gives each its own,
// non-communicating copy, since neither dynamically links the other.
//
// All members are noexcept: the C entry points are noexcept by contract (registration.cc),
// and ice::String's construction is malloc-backed, so no exception can escape.
class Registration
{
private:
    struct RegistryState
    {
        TF_Registration* ops = nullptr;
        void* plugin_context = nullptr;

        RegistryState() noexcept
        {
            // Real status channel instead of nullptr: if init fails we must not silently
            // pretend the registry is usable.
            ice::Status status;
            init_registration(&ops, &plugin_context, status.get_handle());
            if (!status.ok() || !ops || ops->struct_size < sizeof(TF_Registration)) {
                ops = nullptr;
                plugin_context = nullptr;
            }
        }
    };

    static const RegistryState& state() noexcept
    {
        static RegistryState s;
        return s;
    }

public:
    static void register_value(const char* type, const char* name, void* value) noexcept
    {
        const auto& s = state();
        if (s.ops && s.ops->register_op) {
            // The C ABI's register_op takes TF_TString* type/name — adapt the C strings at
            // the boundary line.
            ice::String type_str{type};
            ice::String name_str{name};
            s.ops->register_op(
                s.plugin_context,
                type_str.get_handle(),
                name_str.get_handle(),
                value
            );
        }
    }

    static void* get(const char* type, const char* name) noexcept
    {
        const auto& s = state();
        if (s.ops && s.ops->get) {
            // The C ABI's get/unregister take a TF_String* name — adapt the C string at the
            // boundary line.
            ice::String type_str{type};
            ice::String name_str{name};
            return s.ops->get(s.plugin_context, type_str.get_handle(), name_str.get_handle());
        }
        return nullptr;
    }

    static void unregister(const char* type, const char* name) noexcept
    {
        const auto& s = state();
        if (s.ops && s.ops->unregister) {
            ice::String type_str{type};
            ice::String name_str{name};
            s.ops->unregister(s.plugin_context, type_str.get_handle(), name_str.get_handle());
        }
    }
};

} // namespace ice::sonic

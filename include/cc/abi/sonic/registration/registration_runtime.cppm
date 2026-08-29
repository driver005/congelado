module;

#include "c/extern/registration/registration.h"
#include "c/intern/tf_status.h"

export module cc_abi_sonic_registration:registration_runtime;

export namespace ice::sonic {

// Thin wrapper around c/extern/registration/registration.h's generic named-value registry —
// the genuinely process-wide (linkshared, see include/c/BUILD's registration_shared target)
// storage that lets a dynamically loaded plugin .so register a factory function pointer
// visible to the host process. A plain in-process singleton would break here: statically
// linking the same singleton code into both the host and a plugin .so gives each its own,
// non-communicating copy, since neither dynamically links the other.
class RegistrationRuntime
{
private:
    struct RegistryState
    {
        TF_RegistrationOps* ops = nullptr;
        void* plugin_context = nullptr;

        RegistryState()
        {
            init_registration(&ops, &plugin_context, nullptr);
        }
    };

    static const RegistryState& state()
    {
        static RegistryState s;
        return s;
    }

public:
    static void register_value(const char* type, const char* name, void* value)
    {
        const auto& s = state();
        if (s.ops && s.ops->register_op) {
            s.ops->register_op(s.plugin_context, type, name, value);
        }
    }

    static void* get(const char* type, const char* name)
    {
        const auto& s = state();
        if (s.ops && s.ops->get) {
            return s.ops->get(s.plugin_context, type, name);
        }
        return nullptr;
    }

    static void unregister(const char* type, const char* name)
    {
        const auto& s = state();
        if (s.ops && s.ops->unregister) {
            s.ops->unregister(s.plugin_context, type, name);
        }
    }
};

} // namespace ice::sonic

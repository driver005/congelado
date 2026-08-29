module;

#include "c/extern/plugin/registration.h"

export module cc_abi_sonic_plugin:plugin_runtime;

export namespace ice::sonic {

// Non-owning wrapper around a TF_PluginInfo* — TF_PluginInfo has no allocator function of its
// own, it's a plain fixed-size struct, so create() just hands out a pointer to storage this
// class owns rather than heap allocating.
class PluginRuntime
{
public:
    PluginRuntime() :
        m_handle{nullptr}
    {
    }

    explicit PluginRuntime(TF_PluginInfo* handle) :
        m_handle{handle}
    {
    }

    static PluginRuntime create()
    {
        static TF_PluginInfo storage{};
        return PluginRuntime{&storage};
    }

    TF_PluginInfo* get_handle() const
    {
        return m_handle;
    }

private:
    TF_PluginInfo* m_handle;
};

} // namespace ice::sonic

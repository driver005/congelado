// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/plugin/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/plugin/registration.h"

export module cc_ice_extern_plugin_sonic:plugin_info;

import std;

export namespace ice::sonic {

class PluginInfo
{
public:
    PluginInfo() noexcept {}

    explicit PluginInfo(const ::TF_PluginInfo& value) noexcept :
        m_value{value}
    {
    }

    PluginInfo(const char* name, const char* version) noexcept
    {
        m_value.name = name;
        m_value.version = version;
    }

    const char* getName() const noexcept
    {
        return m_value.name;
    }

    void setName(const char* value) noexcept
    {
        m_value.name = value;
    }

    const char* getVersion() const noexcept
    {
        return m_value.version;
    }

    void setVersion(const char* value) noexcept
    {
        m_value.version = value;
    }

    ::TF_PluginInfo* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_PluginInfo* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_PluginInfo m_value{};
};

} // namespace ice::sonic

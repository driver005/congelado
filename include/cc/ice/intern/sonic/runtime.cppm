// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen (runtime support). Re-run `make gen-cc-abi` to regenerate; edits made
// directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"

export module cc_ice_intern_sonic:runtime;

import std;

export namespace ice::sonic {

template<typename OpsStruct, typename HandleStruct>
class Runtime
{
public:
    HandleStruct* get_handle() const noexcept
    {
        return &m_handle;
    }

    const OpsStruct* get_ops() const noexcept
    {
        return m_ops;
    }

    explicit operator bool() const noexcept
    {
        return m_ops != nullptr && m_handle.plugin_data != nullptr;
    }

protected:
    explicit Runtime(const OpsStruct* ops) noexcept :
        m_ops{ops}
    {
    }

    Runtime(const OpsStruct* ops, HandleStruct* handle) noexcept :
        m_ops{ops}
    {
        if (handle != nullptr) {
            m_handle = *handle;
        }
    }

    Runtime(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ::TF_String* type,
        const ::TF_String* provider
    ) noexcept
    {
        void* ops = nullptr;
        registry_ops.get(registry_handle, type, provider, &ops);
        m_ops = static_cast<const OpsStruct*>(ops);
    }

    Runtime(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        HandleStruct* handle,
        const ::TF_String* type,
        const ::TF_String* provider
    ) noexcept
    {
        void* ops = nullptr;
        registry_ops.get(registry_handle, type, provider, &ops);
        m_ops = static_cast<const OpsStruct*>(ops);
        if (handle != nullptr) {
            m_handle = *handle;
        }
    }

    const OpsStruct* m_ops{nullptr};
    mutable HandleStruct m_handle{};
};

} // namespace ice::sonic

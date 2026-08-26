module;

#include "c/extern/cache.h"

export module cc_abi_builder:cache;

import cc_abi_sonic_intern;

export namespace ice::builder {

class CacheBuilder
{
public:
    CacheBuilder() :
        m_handle{TP_CacheNew()}
    {
    }

    ~CacheBuilder()
    {
        TP_CacheDelete(m_handle);
    }

    CacheBuilder(const CacheBuilder&) = delete;
    CacheBuilder& operator=(const CacheBuilder&) = delete;

    CacheBuilder(CacheBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    CacheBuilder& operator=(CacheBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_CacheDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    CacheBuilder& set_get(TP_Cache_GetFn callback)
    {

        TP_Cache_SetGetCallback(m_handle, callback);
        return *this;
    }

    CacheBuilder& set_set(TP_Cache_SetFn callback)
    {

        TP_Cache_SetSetCallback(m_handle, callback);
        return *this;
    }

    CacheBuilder& set_remove(TP_Cache_RemoveFn callback)
    {

        TP_Cache_SetRemoveCallback(m_handle, callback);
        return *this;
    }

    ice::sonic::StringRuntime get_name()
    {
        return ice::sonic::StringRuntime{&m_handle->backend_name};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Cache* get_handle()
    {
        return m_handle;
    }

    const TP_Cache* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Cache* m_handle;
};

} // namespace ice::builder

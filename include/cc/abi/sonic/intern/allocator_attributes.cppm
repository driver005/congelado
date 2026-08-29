module;

#include "c/intern/tf_tensor.h"

export module cc_abi_sonic_intern:allocator_attributes;

import std;

export namespace ice::sonic {

class AllocatorAttributes
{
public:
    AllocatorAttributes() :
        m_on_host(false),
        m_nic_compatible(false),
        m_allocator_id(0)
    {
    }

    AllocatorAttributes(bool on_host, bool nic, int32_t id) :
        m_on_host(on_host),
        m_nic_compatible(nic),
        m_allocator_id(id)
    {
    }

    bool get_on_host() const
    {
        return m_on_host;
    }

    void set_on_host(bool v)
    {
        m_on_host = v;
    }

    bool get_nic_compatible() const
    {
        return m_nic_compatible;
    }

    void set_nic_compatible(bool v)
    {
        m_nic_compatible = v;
    }

    int32_t get_allocator_id() const
    {
        return m_allocator_id;
    }

    void set_allocator_id(int32_t v)
    {
        m_allocator_id = v;
    }

private:
    bool m_on_host;
    bool m_nic_compatible;
    int32_t m_allocator_id;
};

} // namespace ice::sonic

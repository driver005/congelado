module;

export module cc_abi_primitives:allocator_attributes;

import std;

export namespace ice {

class AllocatorAttributes
{
public:
    AllocatorAttributes() noexcept :
        m_on_host(false),
        m_nic_compatible(false),
        m_allocator_id(0)
    {
    }

    AllocatorAttributes(bool on_host, bool nic, int32_t id) noexcept :
        m_on_host(on_host),
        m_nic_compatible(nic),
        m_allocator_id(id)
    {
    }

    bool get_on_host() const noexcept
    {
        return m_on_host;
    }

    void set_on_host(bool v) noexcept
    {
        m_on_host = v;
    }

    bool get_nic_compatible() const noexcept
    {
        return m_nic_compatible;
    }

    void set_nic_compatible(bool v) noexcept
    {
        m_nic_compatible = v;
    }

    int32_t get_allocator_id() const noexcept
    {
        return m_allocator_id;
    }

    void set_allocator_id(int32_t v) noexcept
    {
        m_allocator_id = v;
    }

private:
    bool m_on_host;
    bool m_nic_compatible;
    int32_t m_allocator_id;
};

} // namespace ice

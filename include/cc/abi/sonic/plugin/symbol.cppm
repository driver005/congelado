export module cc_abi_sonic_plugin:symbol;

import std;

export namespace ice::sonic {

// A resolved dynamic-library symbol — call() invokes it as a void(void*) function, passing
// `arg` through. Matches the "host allocates, callee fills through the pointer" contract every
// caller of this class currently needs (e.g. init_plugin-shaped entry points).
class Symbol
{
public:
    explicit Symbol(void* handle) noexcept :
        m_handle{handle}
    {
    }

    void call(void* arg) const noexcept
    {
        // m_handle is a void* carrying a function pointer — std::bit_cast recovers the
        // callable with a compile-time size check instead of a raw reinterpret_cast.
        std::bit_cast<void (*)(void*)>(m_handle)(arg);
    }

    void* get_handle() const noexcept
    {
        return m_handle;
    }

private:
    void* m_handle;
};

} // namespace ice::sonic

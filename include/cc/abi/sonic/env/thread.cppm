module;

#include "c/extern/env/thread.h"

export module cc_abi_sonic_env:thread;

export namespace ice::sonic {

// ThreadView — non-owning wrapper around a `TF_Thread*` handle. Duplicated from
// builder/env/thread.cppm's ThreadView for pattern uniformity — in practice `start_thread()`
// is called and its result joined entirely plugin-side, so this has no real host caller today.
class ThreadView
{
public:
    explicit ThreadView(TF_Thread* handle) :
        m_handle(handle)
    {
    }

    ~ThreadView() = default;

    ThreadView(const ThreadView&) = delete;
    ThreadView& operator=(const ThreadView&) = delete;

    ThreadView(ThreadView&& other) noexcept :
        m_handle(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    ThreadView& operator=(ThreadView&& other) noexcept
    {
        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    void join()
    {
        if (m_handle) {
            join_thread(m_handle);
        }
    }

    // Underlying handle — pass directly to the C ABI
    TF_Thread* get_handle()
    {
        return m_handle;
    }

    const TF_Thread* get_handle() const
    {
        return m_handle;
    }

private:
    TF_Thread* m_handle;
};

} // namespace ice::sonic

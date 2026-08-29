module;

#include "c/extern/env/thread.h"

export module cc_abi_builder_env:thread_view;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

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

inline ThreadView start_thread(
    const TF_ThreadOptions* options,
    const ice::String& thread_name,
    TF_ThreadWorkFn work_func,
    void* param
)
{
    return ThreadView(start_thread(options, thread_name.c_str(), work_func, param));
}

} // namespace ice::builder

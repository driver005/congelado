module;

#include "c/extern/io/response.h"

export module cc_abi_runtime_io:response;

export namespace ice {

// Borrowed wrapper for TP_IO_Response — renamed from today's ResponseView (io/request_response.cppm).
class ResponseRuntime {
  public:
    explicit ResponseRuntime(TP_IO_Response *handle) : m_handle{handle} {}
    ResponseRuntime() : m_handle{nullptr} {}
    ~ResponseRuntime() = default;

    ResponseRuntime(const ResponseRuntime &) = delete;
    ResponseRuntime &operator=(const ResponseRuntime &) = delete;

    ResponseRuntime(ResponseRuntime &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    ResponseRuntime &operator=(ResponseRuntime &&other) noexcept {
        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TP_IO_Response *get_handle() { return m_handle; }
    const TP_IO_Response *get_handle() const { return m_handle; }

  private:
    TP_IO_Response *m_handle;
};

} // namespace ice

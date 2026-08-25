module;

#include "c/extern/io/request.h"

export module cc_abi_runtime_io:request;

export namespace ice {

// Borrowed wrapper for TP_IO_Request — renamed from today's RequestView (io/request_response.cppm),
// which already had exactly this non-owning shape.
class RequestRuntime {
  public:
    explicit RequestRuntime(TP_IO_Request *handle) : m_handle{handle} {}
    RequestRuntime() : m_handle{nullptr} {}
    ~RequestRuntime() = default;

    RequestRuntime(const RequestRuntime &) = delete;
    RequestRuntime &operator=(const RequestRuntime &) = delete;

    RequestRuntime(RequestRuntime &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    RequestRuntime &operator=(RequestRuntime &&other) noexcept {

        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    // Underlying handle — pass directly to the C ABI
    TP_IO_Request *get_handle() { return m_handle; }
    const TP_IO_Request *get_handle() const { return m_handle; }

  private:
    TP_IO_Request *m_handle;
};

} // namespace ice

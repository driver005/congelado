// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"

export module cc_ice_extern_io_sonic:socket_status;

import std;

export namespace ice::sonic {

class SocketStatus
{
public:
    enum Value : std::underlying_type_t<::TFSocketStatus>
    {
        k_errored = ::TF_SOCKET_ERRORED,
        k_valid = ::TF_SOCKET_VALID,
        k_cleanly_disconnected = ::TF_SOCKET_CLEANLY_DISCONNECTED,
        k_would_block = ::TF_SOCKET_WOULD_BLOCK,
        k_timed_out = ::TF_SOCKET_TIMED_OUT
    };

    constexpr SocketStatus() noexcept = default;

    constexpr SocketStatus(::TFSocketStatus value) noexcept :
        m_value{value}
    {
    }

    constexpr SocketStatus(Value value) noexcept :
        m_value{static_cast<::TFSocketStatus>(value)}
    {
    }

    constexpr ::TFSocketStatus get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFSocketStatus() const noexcept
    {
        return m_value;
    }

private:
    ::TFSocketStatus m_value{};
};

} // namespace ice::sonic

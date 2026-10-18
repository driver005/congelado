// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"

export module cc_ice_extern_io_sonic:socket_protocol;

import std;

export namespace ice::sonic {

class SocketProtocol
{
public:
    enum Value : std::underlying_type_t<::TFSocketProtocol>
    {
        k_tcp = ::TF_SOCKET_TCP,
        k_udp = ::TF_SOCKET_UDP,
        k_tls = ::TF_SOCKET_TLS,
        k_quic = ::TF_SOCKET_QUIC
    };

    constexpr SocketProtocol() noexcept = default;

    constexpr SocketProtocol(::TFSocketProtocol value) noexcept :
        m_value{value}
    {
    }

    constexpr SocketProtocol(Value value) noexcept :
        m_value{static_cast<::TFSocketProtocol>(value)}
    {
    }

    constexpr ::TFSocketProtocol get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFSocketProtocol() const noexcept
    {
        return m_value;
    }

private:
    ::TFSocketProtocol m_value{};
};

} // namespace ice::sonic

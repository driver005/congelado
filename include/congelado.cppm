module;

#include <stdio.h>

export module congelado;

import std;
import interfaces;
import shared;
import core_ffi;
import core_contract;
import io_base_flow;
import io_base_socket;
import io_base_leverage;
import hashmap;

export namespace app {

class Server {
  public:
    // protocol owns transport config and connection lifecycle.
    // protocol must outlive Server.
    explicit Server(interfaces::IProtocol &protocol)
        : m_protocol{protocol}, m_contract_group{},
          m_thread_pool{m_contract_group, static_cast<std::size_t>(protocol.get_bind_threads())}, m_leverager{},
          m_socket_flow{make_socket_flow()} {}

  private:
    inline io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS>
    make_socket_flow() {
        printf("Hello, Congelado!\n");
        io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> flow{
            io::base::socket::Endpoint{std::string{m_protocol.get_bind_host()}, m_protocol.get_bind_port()},
            m_leverager, m_contract_group};
        flow.add_on_accept([&](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
            std::println("New connection accepted, starting {} flow", m_protocol.get_protocol_name());
            return m_protocol.on_connect(std::move(send), std::move(close));
        });
        flow.build();
        return flow;
    }

    // m_protocol declared first — bind_threads() used in ContractThreadPool init
    interfaces::IProtocol &m_protocol;
    core::contract::ContractGroup<> m_contract_group;
    core::contract::ContractThreadPool<> m_thread_pool;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> m_socket_flow;
};


// // NOTE: DO NOT REMOVE AND MAKE FIRST PLUGIN!
// class MyCustomFileLogger : public interfaces::ILogger {
//   private:
//     std::string filepath;
//
//   public:
//     MyCustomFileLogger(std::string path) : filepath(std::move(path)) {}
//
//     [[nodiscard]] std::string_view name() const noexcept override { return "MyCustomFileLogger"; }
//
//     std::string initialize() override {
//         return std::format("Settings: Custom logger initialized saving to {}", filepath);
//     }
//
//     void write(shared::LogLevel level, std::string_view message) noexcept override {
//         auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
//         std::println("[{:%H:%M:%S}] [{}]: {}", time, to_string(level), message);
//     }
//
//     void error(std::string_view message) override {
//         auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
//         throw std::runtime_error(std::format("{} - [{:%H:%M:%S}] [ERROR]: {}", name(), time, message));
//     }
// };
//
} // namespace app

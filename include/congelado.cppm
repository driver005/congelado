module;

#include <stdio.h>

export module congelado;

import std;
import interfaces;
import shared;
import io_layer_http2;
import core_contract;
import io_base_flow;
import io_base_socket;
import io_base_leverage;
import hashmap;


export namespace app {

class Server {
  public:
    Server()
        : m_contract_group{}, m_thread_pool{m_contract_group, 1}, m_leverager{}, m_table{},
          m_socket_flow{make_socket_flow()} {}

  private:
    inline io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS>
    make_socket_flow() {
        printf("Hello, Congelado!\n");
        io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> flow{
            io::base::socket::Endpoint{"localhost", 8080}, m_leverager, m_contract_group};
        flow.add_on_accept([&](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
            std::println("New connection accepted, creating HTTP/2 flow");
            return m_table.emplace_back(std::make_unique<io::layer::http2::Flow>(std::move(send), std::move(close)))
                ->on_read();
        });
        flow.build();

        return flow;
    }

    core::contract::ContractGroup<> m_contract_group;
    core::contract::ContractThreadPool<> m_thread_pool;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::deque<std::unique_ptr<io::layer::http2::Flow>> m_table;
    io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> m_socket_flow;
};


// NOTE: DO NOT REMOVE AND MAKE FIRST PLUIGN!
class MyCustomFileLogger : public interfaces::ILogger {
  private:
    std::string filepath;

  public:
    MyCustomFileLogger(std::string path) : filepath(std::move(path)) {}

    std::string name() const override { return "MyCustomFileLogger"; }

    std::string initialize() override {
        return std::format("Settings: Custom logger initialized saving to {}", filepath);
    }

    void write(shared::LogLevel level, std::string_view message) noexcept override {
        auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        std::println("[{:%H:%M:%S}] [{}]: {}", time, to_string(level), message);
    }

    void error(std::string_view message) override {
        auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        throw std::runtime_error(std::format("{} - [{:%H:%M:%S}] [ERROR]: {}", name(), time, message));
    }
};

} // namespace app

// --- Client usage example ---
//
// io::layer::http2::Session session{send_cb, close_cb};
//
// // Simple GET (no body) — HEADERS + END_STREAM, stream goes IDLE → HALF_CLOSED_LOCAL
// auto req = io::shared::http::HttpRequest::get(1, "/api/users")
//     .with_authority("example.com")
//     .with_scheme("https")
//     .with_user_agent("congelado/1.0");
// session.send(req);
//
// // POST with body — HEADERS frame (OPEN), then DATA + END_STREAM (HALF_CLOSED_LOCAL)
// auto post_req = io::shared::http::HttpRequest::post(1, "/api/users")
//     .with_authority("example.com")
//     .with_scheme("https")
//     .with_content_type("application/json");
// // populate post_req body via the BufferView from post_req.get_body() before calling send
// session.send(post_req);

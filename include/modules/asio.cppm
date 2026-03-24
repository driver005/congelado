module;

// 1. Critical Defines
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE 1
#endif
// #ifndef ASIO_SEPARATE_COMPILATION
// #define ASIO_SEPARATE_COMPILATION 1
// #endif

// 2. Comprehensive Includes in Global Fragment
#include <asio.hpp>
#include <asio/bind_executor.hpp>
#include <asio/signal_set.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>

export module asio;

// 3. Use explicit global scope (::asio) to prevent compiler recursion
export namespace asio {
using ::asio::any_io_executor;
using ::asio::async_write;
using ::asio::bind_executor;
using ::asio::buffer;
using ::asio::error_code;
using ::asio::executor_work_guard;
using ::asio::io_context;
using ::asio::make_strand;
using ::asio::make_work_guard;
using ::asio::post;
using ::asio::signal_set;
using ::asio::socket_base;
using ::asio::strand;

namespace ip {
using ::asio::ip::address; // Added to fix your previous 'to_string' error
using ::asio::ip::tcp;
using ::asio::ip::v6_only;
} // namespace ip

namespace ssl {
using ::asio::ssl::context;
using ::asio::ssl::stream;
using ::asio::ssl::stream_base;
} // namespace ssl

namespace error {
using ::asio::error::operation_aborted;
}
} // namespace asio

module modules.asio;
@nogc nothrow:

// PORT-NOTE: asio is a C++ library with no direct D equivalent.
// This module is a stub that documents the asio symbols used by the C++ codebase.
// In the D port, asio functionality (async I/O, executors, strands, SSL) should be
// replaced by D's native facilities: core.sys.posix I/O, io_uring via the leverage
// layer, and a TLS wrapper over OpenSSL directly. See modules/openssl.d for SSL types.
//
// Symbols re-exported from the C++ asio module:
//   asio::any_io_executor
//   asio::async_write
//   asio::bind_executor
//   asio::buffer
//   asio::error_code
//   asio::executor_work_guard
//   asio::io_context
//   asio::make_strand
//   asio::make_work_guard
//   asio::post
//   asio::signal_set
//   asio::socket_base
//   asio::strand
//   asio::ip::address
//   asio::ip::tcp
//   asio::ip::v6_only
//   asio::ssl::context
//   asio::ssl::stream
//   asio::ssl::stream_base
//   asio::error::operation_aborted
//
// 1. Critical Defines (C++ side)
//   ASIO_STANDALONE 1
// #ifndef ASIO_SEPARATE_COMPILATION
// #define ASIO_SEPARATE_COMPILATION 1
// #endif

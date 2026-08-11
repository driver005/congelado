export module congelado_worker:consts;

import std;

export namespace congelado::worker::consts {

// Default worker config values, applied when a key is absent from the config file (see
// worker_main.cc's value_or(...) calls). One place to tune every default.

// Delay between failed engine-connect attempts, in milliseconds.
inline constexpr std::uint32_t connect_retry_delay_ms = 1000;

// Overall engine-connect deadline, in milliseconds. 0 means retry forever, never give up.
inline constexpr std::uint32_t connect_timeout_ms = 0;

} // namespace congelado::worker::consts

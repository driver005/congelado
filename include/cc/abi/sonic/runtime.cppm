export module cc_abi_sonic;

// Root-level standalone partitions (same package)
export import :cache;
export import :cron;
export import :database;
export import :events;
export import :logger;
export import :worker_manager;
export import :ops;
export import :worker_orchestrator;
export import :payload;
export import :profiler;
export import :protocol_server_runtime;
export import :protocol_runtime;
export import :python;
export import :search_query_runtime;
export import :search_runtime;
export import :serde;
export import :worker;

// Subdirectory parent modules are separate modules, imported normally
import cc_abi_sonic_intern;
import cc_abi_sonic_otel;
import cc_abi_sonic_io;
import cc_abi_sonic_filesystem;
import cc_abi_sonic_env;
import cc_abi_sonic_generator;

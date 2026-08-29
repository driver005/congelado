export module cc_abi_sonic;

// Root-level standalone partitions (same package)
export import :python;

// Subdirectory parent modules are separate modules, imported normally
import cc_abi_sonic_intern;
import cc_abi_sonic_otel;
import cc_abi_sonic_io;
import cc_abi_sonic_filesystem;
import cc_abi_sonic_env;
import cc_abi_sonic_plugin;
import cc_abi_sonic_generator;
import cc_abi_sonic_protocol;
import cc_abi_sonic_search;
import cc_abi_sonic_cache;
import cc_abi_sonic_cron;
import cc_abi_sonic_database;
import cc_abi_sonic_events;
import cc_abi_sonic_logger;
import cc_abi_sonic_payload;
import cc_abi_sonic_serde;
import cc_abi_sonic_worker;
import cc_abi_sonic_manager;
import cc_abi_sonic_orchestrator;
import cc_abi_sonic_profiler;

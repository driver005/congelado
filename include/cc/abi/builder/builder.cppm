export module cc_abi_builder;

// Root-level standalone partitions (same package)
export import :python;

// Subdirectory parent modules are separate modules, imported normally
import cc_abi_builder_intern;
import cc_abi_builder_otel;
import cc_abi_builder_profiler;
import cc_abi_builder_io;
import cc_abi_builder_filesystem;
import cc_abi_builder_env;
import cc_abi_builder_generator;
import cc_abi_builder_cache;
import cc_abi_builder_cron;
import cc_abi_builder_database;
import cc_abi_builder_events;
import cc_abi_builder_logger;
import cc_abi_builder_payload;
import cc_abi_builder_serde;
import cc_abi_builder_worker;
import cc_abi_builder_manager;
import cc_abi_builder_orchestrator;
import cc_abi_builder_protocol;
import cc_abi_builder_search;

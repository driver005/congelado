export module core_heart:context;

import std;
import io_shared;
import core_server;

export namespace core::heart {

// Aggregates shared resources that plugins read/write during on_load.
// Owned by App::run() and lives for the duration of the process.
// Passed to every plugin via CongeladoHostCallbacks so plugins can register
// routes, subscribe to events, etc. without coupling to App internals.
// Add fields here as the platform grows.
struct AppContext {
    core::server::RouterContext<io::shared::http::Protocol> router;

    // Returns router as opaque void* for CongeladoHostCallbacks::router_ctx.
    // Plugins cast back to RouterContext<io::shared::http::Protocol>* to add routes.
    [[nodiscard]] void* router_ptr() noexcept { return &router; }
};

} // namespace core::heart

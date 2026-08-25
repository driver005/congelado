export module worker_runtime;

export import :context;
export import :poll;
export import :execution;
export import :status;
export import :routes;

#ifdef CONGELADO_TEST
import std;
import interfaces;
import core_router;
import utils_openapi;
import boost.ut;

// This file itself is pure re-export plumbing — no branches, no state, nothing partition-local
// to unit test. Every partition's own behavior is covered in its own file (context.cppm,
// poll.cppm, execution.cppm, status.cppm, routes.cppm). What IS specific to this primary module
// interface unit is the export surface itself — that every partition's public symbols actually
// come through when a consumer only ever writes `import worker_runtime;` (never a direct
// partition import), which is exactly how bin/external_worker_manager.cc consumes this module.
namespace worker::worker_runtime_module_tests {
using namespace boost::ut;

suite<"worker_runtime module export surface"> worker_runtime_export_suite = [] {
    "WorkerContext, StatusHandler, and register_routes are all reachable through a bare `import worker_runtime;`"_test =
        [] {
            WorkerContext ctx;
            ctx.set_worker_id("worker-1");
            StatusHandler::bind(ctx);

            core::router::RouterContext<> router;
            auto routes_before = utils::openapi::Registry::get_routes().size();
            register_routes(router);
            expect(utils::openapi::Registry::get_routes().size() > routes_before);
        };
};

} // namespace worker::worker_runtime_module_tests
#endif

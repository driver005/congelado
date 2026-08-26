// NOLINTBEGIN
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int32_t type_index;
        uint32_t zero_padding;

        union
        {
            int64_t v_int64;
            double v_float64;
            void* v_ptr;
            const char* v_cstr;
        };
    } CongeladoAny;

#define CG_NONE         0
#define CG_INT          1
#define CG_BOOL         2
#define CG_FLOAT        3
#define CG_PTR          4
#define CG_STR          5
#define CG_MAP          64
#define CG_ARRAY        69
#define CG_MAP_HANDLE   70
#define CG_ARRAY_HANDLE 71

    typedef void (*congelado_log_fn)(void* ctx, int level, const char* msg, size_t len);
    typedef void (*congelado_sched_fn)(void* ctx);

    /* ── Universal plugin-call ABI ──────────────────────────────────────────────
     * Replaces the old per-capability symbols (congelado_logger_write/_error,
     * congelado_protocol_get, congelado_storage_get) with one dlsym'd entrypoint,
     * congelado_call(type, action, args, args_count) -> CongeladoAny. GET actions return a
     * CG_PTR to the plugin's own C++ interface (interfaces::ILogger/IDatabase/IProtocol/
     * ISerdeFormat) — the host casts and calls virtuals on it directly in-process, same as
     * today's storage_get/protocol_get; only WRITE/ERROR (logger) carry real marshaled data,
     * since ILogger is deliberately kept non-templated/ABI-stable. Any CG_STR crossing this
     * ABI is null-terminated (matches how STRING_FN symbols already behave). */
    typedef enum
    {
        CONGELADO_RUN_LOGGER = 0,
        CONGELADO_RUN_SERDE = 1,
        CONGELADO_RUN_STORAGE = 2,
        CONGELADO_RUN_PROTOCOL = 3,
        CONGELADO_RUN_BRIDGE = 4,  /* Python/Lua FFI bridges — plugin-like, same as SERDE */
        CONGELADO_RUN_OTEL = 5,    /* OpenTelemetry provider (traces/metrics/logs) — same GET-a-
                                    * native-interface-pointer shape as SERDE/STORAGE/PROTOCOL, one
                                    * IOtelProvider* covering all three optional signals */
        CONGELADO_RUN_OPENAPI = 6, /* OpenAPI document/client-SDK generator — same GET-a-native-
                                    * interface-pointer shape as SERDE/STORAGE/PROTOCOL/OTEL, one
                                    * IOpenApiGenerator* */
        CONGELADO_RUN_SEARCH = 7,  /* Pluggable read-model search provider — same GET-a-native-
                                    * interface-pointer shape as STORAGE, one ISearchProvider* */
        CONGELADO_RUN_EVENTS = 8,  /* Outbound event-bus sink — same GET-a-native-interface-pointer
                                    * shape as STORAGE/SEARCH, one IEventSink*. Unlike
                                    * STORAGE/SEARCH,  resolution happens only in the post-build()
                                    * walk (no host  callback field needed) since nothing needs this
                                    * pointer during  a plugin's own on_load. */
        CONGELADO_RUN_CACHE = 9,   /* Pluggable read-through cache backend (interfaces::ICache) —
                                    * same GET-a-native-interface-pointer shape as STORAGE/SEARCH, one
                                    * ICache*. Same "resolve before build()" story as STORAGE/SEARCH
                                    * (see cache_ctx below) since Connector::set_cache() needs to be
                                    * called during a plugin's own on_load. */
        CONGELADO_RUN_CRON =
            10, /* Pluggable scheduling engine (interfaces::ICron) — same GET-a-
                 * native-interface-pointer shape as STORAGE/SEARCH/CACHE, one ICron*.
                 * Same "resolve before build()" story (see cron_ctx below) since the
                 * engine plugin's on_load installs its fire callback and seeds its
                 * existing schedules into the backend during its own on_load. */
        CONGELADO_RUN_WORKER_MANAGER =
            11,                    /* Pluggable worker supervisor (interfaces::IWorkerManager) —
                                    * same GET-a-native-interface-pointer shape as CRON, one
                                    * IWorkerManager*. Resolved before build() (see
                                    * worker_manager_ctx below) so the host can drive worker
                                    * lifecycle (spawn/start on ready, shutdown_all on stop).
                                    * Task dispatch stays engine-side — this only supervises. */
        CONGELADO_RUN_WORKER = 12, /* A single task worker (interfaces::IWorker) — same
                                    * GET-a-native- interface-pointer shape as WORKER_MANAGER, one
                                    * IWorker*. Unlike the single-active capabilities, a deployment
                                    * loads MANY worker plugins; the worker host collects every one
                                    * and keys them by get_task_type(). Resolved by the host that
                                    * owns the worker manager, not into a host-callback field. */
        CONGELADO_RUN_APP_DEFS =
            13, /* An app's code-built defs (interfaces::IAppDefs) — GET returns an
                 * IAppDefs* whose get_task_defs()/get_workflow_defs() hand back
                 * serialized (JSON) defs the worker host registers with the engine
                 * on load, the C++-builder counterpart to the def-file path. Many
                 * app plugins may export this; the host collects every one, same
                 * multi-plugin shape as WORKER. */
        CONGELADO_RUN_WORKER_ORCHESTRATOR =
            14, /* Pluggable engine-side dispatch/orchestration backend
                 * (interfaces::IWorkerOrchestrator) — same GET-a-native-
                 * interface-pointer shape as WORKER_MANAGER, one
                 * IWorkerOrchestrator*. Resolved before build() (see
                 * worker_orchestrator_ctx below) so the engine plugin's
                 * on_load can wire it in. The engine-side counterpart to
                 * WORKER_MANAGER: it owns task dispatch, not worker
                 * lifecycle. */
        CONGELADO_RUN_WORKFLOW_ORCHESTRATOR =
            15, /* Pluggable workflow-lifecycle backend
                 * (interfaces::IWorkflowOrchestrator) — same GET-a-
                 * native-interface-pointer shape as
                 * WORKER_ORCHESTRATOR, one IWorkflowOrchestrator*.
                 * Resolved before build() (see
                 * workflow_orchestrator_ctx below). Owns the workflow
                 * DAG lifecycle (start/pause/resume/advance), the
                 * counterpart to WORKER_ORCHESTRATOR's task dispatch. */
        CONGELADO_RUN_PAYLOAD_STORAGE =
            16, /* Pluggable externalized payload storage
                 * (interfaces::IExternalPayloadStorage) — same GET-a-native-
                 * interface-pointer shape as STORAGE/CACHE, one
                 * IExternalPayloadStorage*. Resolved before build() (see
                 * payload_storage_ctx below) so the engine plugin's on_load
                 * wires the resolved backend (local-disk, S3, …) in. */
    } CongeladoRunType;

    typedef enum
    {
        CONGELADO_ACTION_WRITE = 0, /* LOGGER: args = [level:CG_INT, msg:CG_STR] */
        CONGELADO_ACTION_ERROR = 1, /* LOGGER: args = [msg:CG_STR] */
        CONGELADO_ACTION_GET = 2, /* SERDE/STORAGE/PROTOCOL/BRIDGE/OTEL/OPENAPI/SEARCH/EVENTS/CACHE:
                                   * args = [], returns CG_PTR (the plugin's native interface
                                   * pointer, e.g. IBridge or IOtelProvider or IOpenApiGenerator or
                                   * ISearchProvider or IEventSink or ICache, cast in-process) or
                                   * CG_NONE */
        CONGELADO_ACTION_GET_NATIVE_HANDLE =
            3, /* BRIDGE only: args = [], returns CG_PTR (e.g. a
                * lua_State*) or CG_NONE if the bridge has no
                * separate native handle to expose (Python's
                * interpreter is process-global, no handle needed) */
    } CongeladoRunAction;

    typedef CongeladoAny (*congelado_call_fn)(
        CongeladoRunType type,
        CongeladoRunAction action,
        const CongeladoAny* args,
        size_t args_count
    );

    typedef struct CongeladoHostCallbacks
    {
        congelado_log_fn log;
        congelado_sched_fn schedule;
        void* router_ctx;
        void* controller_ctx;
        void* registry_ctx; /* core::contract::ContractRegistry*, host-owned — a plugin registers a
                             * Contract<> it creates via controller_ctx into this so it gets
                             * released automatically at host shutdown (AppContext::stop()) instead
                             * of the plugin managing its own release. Same lifetime the host's own
                             * connector_ctx contract already gets. */
        void* leverager_ctx;
        void* database_ctx;   /* resolved interfaces::IDatabase*, if a storage-capable plugin was
                               * already opened by the time this gets set — populated before build()
                               * runs so an on_load hook (e.g. the engine plugin's) can read it back
                               * out, cast, and wire it into its own connector. NULL if none resolved.
                               */
        void* lua_bridge_ctx; /* resolved interfaces::IBridge* for the "lua" runtime specifically
                               * (not just any bridge), same "resolve before build()" reasoning as
                               * database_ctx — the engine plugin's Lua condition evaluator needs
                               * this during its own on_load, and the normal bridge-broadcast walk
                               * only runs after build(), too late for that. NULL if no lua_bridge
                               * plugin was found. */
        void* ctx; /* generic userdata pointer passed as the first argument to `log` — a plugin's
                    * congelado_log_fn callback signature takes this back as its own `ctx` param. */
        void* search_ctx; /* resolved interfaces::ISearchProvider*, if a search-capable plugin was
                           * already opened by the time this gets set — same "resolve before
                           * build()" reasoning as database_ctx/lua_bridge_ctx, so the engine
                           * plugin's on_load can wire it into EngineContext for its
                           * terminal-transition projector. NULL if none resolved. */
        void* cache_ctx;  /* resolved interfaces::ICache*, if a cache-capable plugin was already
                           * opened by the time this gets set — same "resolve before build()"
                           * reasoning as database_ctx/search_ctx, so the engine plugin's on_load can
                           * wire it into its own Connector via set_cache(). NULL if none resolved
                           * (Connector's own in-process LocalCache fallback still applies). */
        void* connector_ctx; /* connector::Connector* registered with the host's ContractGroup,
                              * populated before build() so the engine plugin can use the shared
                              * connector instead of creating its own. NULL if not set up. */
        void* cron_ctx; /* resolved interfaces::ICron*, if a cron-capable plugin was already opened
                         * by the time this gets set — same "resolve before build()" reasoning as
                         * search_ctx/cache_ctx, so the engine plugin's on_load can install its fire
                         * callback and seed existing WorkflowSchedules into the backend. NULL if
                         * none resolved (schedules then never fire). */
        void* worker_manager_ctx;        /* resolved interfaces::IWorkerManager*, if a
                                          * worker-manager-capable plugin was already opened by the time
                                          * this gets set — same "resolve before build()" reasoning as
                                          * cron_ctx, so the host (ServerRunner) can drive worker lifecycle
                                          * after plugins go ready and tear workers down at shutdown. NULL
                                          * if no worker-manager plugin was found. */
        void* worker_orchestrator_ctx;   /* resolved interfaces::IWorkerOrchestrator*, if a
                                          * worker-orchestrator-capable plugin was already opened by
                                          * the   time this gets set — same "resolve before build()"
                                          * reasoning   as     worker_manager_ctx, so the engine
                                          * plugin's on_load can   route     dispatch/orchestration
                                          * through it. NULL if none found,   in which     case the
                                          * engine keeps its built-in Orchestrator. */
        void* workflow_orchestrator_ctx; /* resolved interfaces::IWorkflowOrchestrator*, if a
                                          * workflow-orchestrator-capable plugin was already opened
                                          * by the time this gets set — same "resolve before
                                          * build()" reasoning as worker_orchestrator_ctx, so the
                                          * engine plugin's on_load can wire the workflow-lifecycle
                                          * backend in. NULL if none found. */
        void* payload_storage_ctx;       /* resolved interfaces::IExternalPayloadStorage*, if a
                                          * payload-storage-capable plugin was already opened by the time
                                          * this       gets set — same "resolve before build()" reasoning
                                          * as       database_ctx, so the engine plugin's on_load can wire
                                          * the       backend       into EngineContext. NULL if none
                                          * found. */
        void* curl_multi_ctx; /* io::base::curl::MultiDriver*, host-owned, process-wide — drives
                               * libcurl's multi interface off the shared io_uring leverager
                               * (CURLMOPT_SOCKETFUNCTION/TIMERFUNCTION ->
                               * Leverager::poll()/timeout()), so workers with no async transport of
                               * their own (email/llm, libcurl's easy interface) can submit a CURL*
                               * easy handle and get a callback on completion without ever blocking
                               * a thread. NULL if the host didn't stand one up. */
        void* client_protocol_ctx; /* interfaces::IProtocol<io::layer::http2::Server>*, host-owned,
                                    * outlives the workers. The `client`/`client_pool` workers call
                                    * get_client(register.make_dispatch()) on it themselves to own
                                    * their own outbound HTTP/2 client + response correlation
                                    * (core::client:: Register), rather than receiving a
                                    * pre-connected client from the host. NULL if no downstream
                                    * endpoint is configured. */
    } CongeladoHostCallbacks;

    typedef struct CongeladoConfigView
    {
        const char* const* keys;
        const char* const* values;
        size_t count;
    } CongeladoConfigView;

    /* ── Plugin C ABI ────────────────────────────────────────────────────────────
     * Every plugin .so must export congelado_init with C linkage.
     * The host calls it after dlopen with (CongeladoHostCallbacks*, CongeladoConfigView*).
     * Plugins may also export optional lifecycle and metadata symbols.          */
    /* Plugins implement:
     *   int  congelado_init(const CongeladoHostCallbacks* host, const CongeladoConfigView* cfg);
     *   const char* congelado_type(void);   // returns "plugin" or "worker"
     *   void congelado_plugin_on_unload(void);
     *   void congelado_plugin_on_ready(void);
     *   const char* congelado_plugin_name(void);
     *   const char* congelado_plugin_version(void);
     */

#ifdef __cplusplus
}
#endif
// NOLINTEND

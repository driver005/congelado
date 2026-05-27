// NOLINTBEGIN

#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Host → Plugin callbacks ─────────────────────────────────────────────────
   Passed to congelado_on_load(). Built as libffi closures by the bridge —
   real callable C function pointers with captured bridge context.
   router_ctx: opaque shared RouterContext<Protocol>* — cast in plugin to add routes. */

typedef void (*CongeladoLogFn)(void *ctx, int level, const char *msg, size_t len);
typedef void (*CongeladoScheduleFn)(void *ctx);

typedef struct CongeladoHostCallbacks {
    CongeladoLogFn      log;
    CongeladoScheduleFn schedule;
    void               *router_ctx;
    void               *ctx;
} CongeladoHostCallbacks;

/* Read-only view of one plugin's config section.
   Valid only for the duration of on_load(). Do not store pointers. */
typedef struct CongeladoConfigView {
    const char *const *keys;
    const char *const *values;
    size_t             count;
} CongeladoConfigView;

/* Capability bitmask — returned by congelado_capabilities() */
#define CONGELADO_CAP_LOGGER   1u
#define CONGELADO_CAP_PROTOCOL 2u
#define CONGELADO_CAP_CUSTOM   4u

/* ── Plugin → Bridge ABI ─────────────────────────────────────────────────────
   Every plugin .so exports a flat set of C symbols. The bridge discovers them
   at load time with dlsym — missing optional symbols are silently skipped.

   Required:
     const char *congelado_plugin_name()
     const char *congelado_plugin_version()
     uint32_t    congelado_capabilities()          bitmask of CONGELADO_CAP_*

   Lifecycle (optional, but usually present):
     void  congelado_on_load(const CongeladoHostCallbacks*, const CongeladoConfigView*)
     void  congelado_on_unload()

   Logger capability (when CONGELADO_CAP_LOGGER is set):
     void  congelado_logger_write(int level, const char *msg, size_t len)
     void  congelado_logger_write_error(const char *msg, size_t len)

   Protocol capability (when CONGELADO_CAP_PROTOCOL is set):
     void *congelado_protocol_get()                returns IProtocol* as void*

   C++ plugins use plugin_api.hpp + CONGELADO_PLUGIN(ClassName) — zero hand-written C. */

#ifdef __cplusplus
}
#endif

// NOLINTEND

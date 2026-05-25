#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Capability IDs — must stay in sync with core::ffi::Cap in bridge.cppm */
typedef enum CongeladoCap {
    CONGELADO_CAP_LOGGER   = 0,
    CONGELADO_CAP_PROTOCOL = 1,
    CONGELADO_CAP_CUSTOM   = 2,
} CongeladoCap;

/* Function pointer types for CongeladoHostCallbacks */
typedef void (*CongeladoLogFn)(void* ctx, int level, const char* msg, size_t len);
typedef void (*CongeladoScheduleFn)(void* ctx);

/* Host-side callbacks given to the plugin at on_load time.
   Built as libffi closures — real callable C function pointers. */
typedef struct CongeladoHostCallbacks {
    CongeladoLogFn      log;
    CongeladoScheduleFn schedule;
    void* ctx;
} CongeladoHostCallbacks;

/* Read-only view of one plugin's config section.
   Valid only for the duration of on_load(). Do not store pointers. */
typedef struct CongeladoConfigView {
    const char* const* keys;
    const char* const* values;
    size_t count;
} CongeladoConfigView;

/* Logger capability vtable — returned by get_capability(CONGELADO_CAP_LOGGER) */
typedef struct CongeladoLoggerCap {
    void (*write)(void* self, int level, const char* msg, size_t len);
    void (*write_error)(void* self, const char* msg, size_t len);
    void* self;
} CongeladoLoggerCap;

/* Protocol capability vtable — placeholder, defined when IProtocol is C-ified.
   For now get_capability(CONGELADO_CAP_PROTOCOL) returns interfaces::IProtocol* cast to void*. */
typedef struct CongeladoProtocolCap CongeladoProtocolCap;

/* Main plugin descriptor — any language that can produce a .so can fill this */
typedef struct CongeladoPlugin {
    const char* name;
    const char* version;
    void  (*on_load)(void* self, const CongeladoHostCallbacks*, const CongeladoConfigView*);
    void  (*on_unload)(void* self);
    /* Returns pointer to cap-specific vtable, or NULL if capability not supported.
       cap_id is one of CongeladoCap. */
    void* (*get_capability)(void* self, uint32_t cap_id);
    void* self;
} CongeladoPlugin;

/* Every plugin .so must export these two symbols (extern "C" for dlsym).
   C++ plugins use plugin_api.hpp + CONGELADO_PLUGIN(ClassName) — zero hand-written C. */
/* CongeladoPlugin* congelado_get_plugin();                  */
/* void             congelado_destroy_plugin(CongeladoPlugin*); */

#ifdef __cplusplus
}
#endif

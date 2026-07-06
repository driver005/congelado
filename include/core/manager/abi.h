// clang-format off
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t type_index;
    uint32_t zero_padding;
    union {
        int64_t v_int64;
        double v_float64;
        void *v_ptr;
        const char *v_cstr;
    };
} CongeladoAny;

#define CG_NONE 0
#define CG_INT 1
#define CG_BOOL 2
#define CG_FLOAT 3
#define CG_PTR 4
#define CG_STR 5
#define CG_MAP 64
#define CG_ARRAY 69
#define CG_MAP_HANDLE 70
#define CG_ARRAY_HANDLE 71

typedef void (*congelado_log_fn)(void *ctx, int level, const char *msg, size_t len);
typedef void (*congelado_sched_fn)(void *ctx);

typedef struct CongeladoHostCallbacks {
    congelado_log_fn   log;
    congelado_sched_fn schedule;
    void              *router_ctx;
    void              *controller_ctx;
    void              *leverager_ctx;
    void              *ctx;
} CongeladoHostCallbacks;

typedef struct CongeladoConfigView {
    const char *const *keys;
    const char *const *values;
    size_t             count;
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
// clang-format on

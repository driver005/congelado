#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_ABI_MAJOR 1
#define PLUGIN_ABI_MINOR 0

typedef enum PluginKind {
    PLUGIN_AUTONOMOUS = 0,
    PLUGIN_REACTIVE = 1,
} PluginKind;

typedef struct OpaqueEvent {
    const char *topic;
    const char *payload_json;
} OpaqueEvent;

typedef struct HostAPI {
    void (*log)(const char *level, const char *message);
} HostAPI;

typedef struct PluginVTable {
    uint32_t abi_major;
    uint32_t abi_minor;
    PluginKind kind;

    void (*on_load)(void *self, const HostAPI *host);
    void (*on_unload)(void *self);

    void (*on_execute)(void *self);
    void (*on_event)(void *self, const OpaqueEvent *ev);

    const char *(*name)(void *self);
    const char *(*version)(void *self);
    const char *(*author)(void *self);
    const char *(*description)(void *self);
    const char **(*subscriptions)(void *self);
    const char **(*dependencies)(void *self);

    const char *last_error;
} PluginVTable;

typedef PluginVTable *(*CreatePluginFn)(void **out_self);
typedef void (*DestroyPluginFn)(PluginVTable *, void *self);

#define PLUGIN_CREATE_SYMBOL "create_plugin"
#define PLUGIN_DESTROY_SYMBOL "destroy_plugin"

#ifdef __cplusplus
}

#endif

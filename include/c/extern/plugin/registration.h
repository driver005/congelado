#ifndef CONGELADO_C_PLUGIN_REGISTRATION_H_
#define CONGELADO_C_PLUGIN_REGISTRATION_H_

#include "c/abi/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // A plugin's own identity, filled in by its TF_InitPlugin.
    typedef struct TF_PluginInfo
    {
        const char* name;
        const char* version;
    } TF_PluginInfo;

    // Initializes a plugin. Must be present in the plugin's shared object — the host dlopen's
    // the library, dlsym's this symbol, and calls it, handing it a pointer to an empty
    // TF_PluginInfo the plugin fills in place. Returns nothing; the host reads the same struct
    // back afterward.
    TF_CAPI_EXPORT void TF_InitPlugin(TF_PluginInfo* plugin_info);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_PLUGIN_REGISTRATION_H_

#ifndef CONGELADO_C_REGISTRATION_H_
#define CONGELADO_C_REGISTRATION_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Generic process-wide named-value registry, shared across every dynamically loaded
    // plugin .so and the host process (this header's implementation is built as its own
    // linkshared target, so there is genuinely one instance of the backing storage — see
    // registration.cc). Used for things like ice::sonic::Generator's in-process
    // lookup of a generator factory function pointer under type="generator", name="stablehlo".

    typedef struct TF_Registration {
        size_t struct_size;
        void (*register_op)(void* plugin_context, const TF_String* type, const TF_String* name, void* value);
        void* (*get)(void* plugin_context, const TF_String* type, const TF_String* name);
        void (*unregister)(void* plugin_context, const TF_String* type, const TF_String* name);
    } TF_Registration;

    TF_CAPI_EXPORT extern void TF_InitRegistration(TF_Registration** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_REGISTRATION_H_

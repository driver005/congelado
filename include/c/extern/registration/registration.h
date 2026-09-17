#ifndef CONGELADO_C_REGISTRATION_H_
#define CONGELADO_C_REGISTRATION_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Generic named-value registry. Used for things like ice::sonic::Generator's in-process lookup of a generator factory function pointer under type="generator", name="stablehlo".
    //
    // TF_Registration is an opaque pointer to one plugin-owned registry instance. Callers that want a single process-wide registry just allocate one handle up front and share it everywhere; nothing here forces that — multiple independent registries are equally valid.
    typedef struct TF_Registration
    {
        void* plugin_data;
    } TF_Registration;

    typedef struct TF_RegistrationOps
    {
        size_t struct_size;
        void (*destroy)(TF_Registration* registration);
        void (*get_name)(TF_Registration* registration, TF_String* out_name);

        void (*register_op)(
            TF_Registration* registration,
            const TF_String* type,
            const TF_String* name,
            void* value
        );
        void (*get)(
            const TF_Registration* registration,
            const TF_String* type,
            const TF_String* name,
            void** out_value
        );
        void (*unregister)(
            TF_Registration* registration,
            const TF_String* type,
            const TF_String* name
        );

    } TF_RegistrationOps;

#define TF_REGISTRATION_STRUCT_SIZE TF_OFFSET_OF_END(TF_RegistrationOps, unregister)

    TF_CAPI_EXPORT void
    create_registration(TF_RegistrationOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_registration(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_REGISTRATION_H_

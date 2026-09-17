#ifndef TENSORFLOW_C_TF_HIVE_HIVE_H_
#define TENSORFLOW_C_TF_HIVE_HIVE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/hive/slot.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Hive — plugin vtable for a type-erased stable-address bucket container (C++26 std::hive/colony equivalent), fixed to element_size bytes per element at creation. Insert/erase are O(1) and never invalidate other elements' slot handles, unlike TF_Vector.

    typedef struct TF_Hive
    {
        void* plugin_data;
    } TF_Hive;

    // Callback type for iterating over a hive's live elements.
    typedef void (*TF_HiveVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via create_hive.
    typedef struct TF_HiveOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_Hive* hive, size_t element_size);

        // Copy one element_size-byte element from value into a newly allocated slot, returning a stable handle to it.
        void (*insert)(TF_Hive* hive, const void* value, TFHiveSlot* out_slot, TF_Status* out_status);

        // Erase the element at slot, invalidating it.
        void (*erase)(TF_Hive* hive, TFHiveSlot* slot, TF_Status* out_status);

        // Non-owning pointer to the element at slot; NULL if slot has been erased.
        void (*get)(const TF_Hive* hive, const TFHiveSlot* slot, const void** out_value, TF_Status* out_status);

        // Call visitor(capture, element) once per live element, in unspecified order.
        void (*for_each)(
            const TF_Hive* hive,
            TF_HiveVisitor visitor,
            void* capture
        );

        // Current live element count.
        void (*size)(const TF_Hive* hive, size_t* out_size);

        void (*destroy)(TF_Hive* hive);

    } TF_HiveOps;

#define TF_HIVE_STRUCT_SIZE TF_OFFSET_OF_END(TF_HiveOps, destroy)

    TF_CAPI_EXPORT void create_hive(TF_HiveOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_hive(void* plugin_context);

    // Real implementation, not declared-only — calls create_hive
    static inline void init_hive(TF_Hive* hive, TF_Status* out_status)
    {
        TF_HiveOps* ops = NULL;
        create_hive(&ops, &hive->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HIVE_HIVE_H_

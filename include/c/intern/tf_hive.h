#ifndef TENSORFLOW_C_TF_HIVE_H_
#define TENSORFLOW_C_TF_HIVE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Hive — plugin vtable for a type-erased stable-address bucket container (C++26 std::hive/colony equivalent), fixed to element_size bytes per element at creation. Insert/erase are O(1) and never invalidate other elements' slot handles, unlike TF_Vector.
    typedef struct TF_HiveOps TF_HiveOps;

    typedef struct TF_Hive
    {
        void* plugin_data;
        const TF_HiveOps* ops;
    } TF_Hive;

    // TF_Hive_Slot is an opaque, stable handle to one element within a TF_Hive.
    typedef struct TF_Hive_Slot TF_Hive_Slot;

    // Callback type for iterating over a hive's live elements.
    typedef void (*TF_HiveVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via create_hive.
    typedef struct TF_HiveOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_Hive* hive, size_t element_size);

        // Copy one element_size-byte element from value into a newly allocated slot, returning a stable handle to it.
        TF_Hive_Slot* (*insert)(TF_Hive* hive, const void* value);

        // Erase the element at slot, invalidating it.
        void (*erase)(TF_Hive* hive, TF_Hive_Slot* slot);

        // Non-owning pointer to the element at slot; NULL if slot has been erased.
        const void* (*get)(const TF_Hive* hive, const TF_Hive_Slot* slot);

        // Call visitor(capture, element) once per live element, in unspecified order.
        void (*for_each)(
            const TF_Hive* hive,
            TF_HiveVisitor visitor,
            void* capture
        );

        // Current live element count.
        size_t (*size)(const TF_Hive* hive);

        void (*destroy)(TF_Hive* hive);

    } TF_HiveOps;

#define TF_HIVE_STRUCT_SIZE TF_OFFSET_OF_END(TF_HiveOps, destroy)

    TF_CAPI_EXPORT void create_hive(TF_HiveOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_hive(void* plugin_context);

    // Real implementation, not declared-only — calls create_hive and fills in hive->ops.
    static inline void init_hive(TF_Hive* hive, TF_Status* status)
    {
        TF_HiveOps* ops = NULL;
        create_hive(&ops, &hive->plugin_data, status);
        hive->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HIVE_H_

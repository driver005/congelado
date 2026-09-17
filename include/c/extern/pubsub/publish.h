#ifndef CONGELADO_C_PUBSUB_PUBLISH_H_
#define CONGELADO_C_PUBSUB_PUBLISH_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TF_PubSub_RetainedFn)(void* user_data, const TF_String* payload, TF_Status* out_status);
    typedef void (*TF_PubSub_AckFn)(void* user_data, TF_Status* out_status);

    typedef struct TF_PubSub_Publish
    {
        void* plugin_data;
    } TF_PubSub_Publish;

    typedef struct TF_PubSub_PublishOps
    {
        size_t struct_size;
        void (*destroy)(TF_PubSub_Publish* publish);

        void (*publish)(
            TF_PubSub_Publish* publish,
            const TF_String* channel,
            const TF_String* payload,
            int retain,
            TF_Status* out_status
        );
        void (*publish_batch)(
            TF_PubSub_Publish* publish,
            const TF_String* channel,
            const TF_Vector* payloads,
            TF_PubSub_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*flush)(
            TF_PubSub_Publish* publish,
            TF_PubSub_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*get_retained)(
            TF_PubSub_Publish* publish,
            const TF_String* channel,
            TF_PubSub_RetainedFn completion,
            void* user_data,
            TF_Status* out_status
        );
    } TF_PubSub_PublishOps;

#define TF_PUBSUB_PUBLISH_STRUCT_SIZE TF_OFFSET_OF_END(TF_PubSub_PublishOps, get_retained)

    TF_CAPI_EXPORT void create_pubsub_publish(
        TF_PubSub_PublishOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_publish(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_PUBLISH_H_

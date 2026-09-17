#ifndef CONGELADO_C_PUBSUB_PUBLISH_H_
#define CONGELADO_C_PUBSUB_PUBLISH_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TFPubSubRetainedFn)(void* user_data, const TF_String* payload, TF_Status* out_status);
    typedef void (*TFPubSubAckFn)(void* user_data, TF_Status* out_status);

    typedef struct TFPubSubPublish
    {
        void* plugin_data;
    } TFPubSubPublish;

    typedef struct TFPubSubPublishOps
    {
        size_t struct_size;
        void (*destroy)(TFPubSubPublish* publish);

        void (*publish)(
            TFPubSubPublish* publish,
            const TF_String* channel,
            const TF_String* payload,
            int retain,
            TF_Status* out_status
        );
        void (*publish_batch)(
            TFPubSubPublish* publish,
            const TF_String* channel,
            const TF_Vector* payloads,
            TFPubSubAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*flush)(
            TFPubSubPublish* publish,
            TFPubSubAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*get_retained)(
            TFPubSubPublish* publish,
            const TF_String* channel,
            TFPubSubRetainedFn completion,
            void* user_data,
            TF_Status* out_status
        );
    } TFPubSubPublishOps;

#define TF_PUBSUB_PUBLISH_STRUCT_SIZE TF_OFFSET_OF_END(TFPubSubPublishOps, get_retained)

    TF_CAPI_EXPORT void create_pubsub_publish(
        TFPubSubPublishOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_publish(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_PUBLISH_H_

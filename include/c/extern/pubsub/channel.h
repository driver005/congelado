#ifndef CONGELADO_C_PUBSUB_CHANNEL_H_
#define CONGELADO_C_PUBSUB_CHANNEL_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFPubSubChannel
    {
        void* plugin_data;
    } TFPubSubChannel;

    typedef struct TFPubSubChannelOps
    {
        size_t struct_size;
        void (*destroy)(TFPubSubChannel* channel);

        void (*create)(TFPubSubChannel* channel, const TF_String* name, const TF_Map* config, TF_Status* out_status);
        void (*drop)(TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status);
        void (*get_config)(TFPubSubChannel* channel, const TF_String* name, TF_Map* out_config, TF_Status* out_status);
        void (*set_config)(TFPubSubChannel* channel, const TF_String* name, const TF_Map* config, TF_Status* out_status);

        void (*get_stats)(TFPubSubChannel* channel, const TF_String* name, TF_Map* out_stats, TF_Status* out_status);
        void (*purge)(TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status);

        void (*set_dead_letter)(TFPubSubChannel* channel, const TF_String* target_channel, const TF_String* dead_letter_channel, TF_Status* out_status);
        void (*list_dead_letters)(TFPubSubChannel* channel, const TF_String* target_channel, TF_Vector* out_payloads, TF_Status* out_status);
        void (*requeue_dead_letter)(TFPubSubChannel* channel, const TF_String* target_channel, const TF_String* payload, TF_Status* out_status);

        void (*list)(TFPubSubChannel* channel, TF_Vector* out_channels, TF_Status* out_status);
    } TFPubSubChannelOps;

#define TF_PUBSUB_CHANNEL_STRUCT_SIZE TF_OFFSET_OF_END(TFPubSubChannelOps, list)

    TF_CAPI_EXPORT void create_pubsub_channel(
        TFPubSubChannelOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_channel(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_CHANNEL_H_

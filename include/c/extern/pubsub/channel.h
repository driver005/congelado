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

    typedef struct TF_PubSub_Channel
    {
        void* plugin_data;
    } TF_PubSub_Channel;

    typedef struct TF_PubSub_ChannelOps
    {
        size_t struct_size;
        void (*destroy)(TF_PubSub_Channel* channel);

        void (*create)(TF_PubSub_Channel* channel, const TF_String* name, const TF_Map* config, TF_Status* out_status);
        void (*delete)(TF_PubSub_Channel* channel, const TF_String* name, TF_Status* out_status);
        void (*get_config)(TF_PubSub_Channel* channel, const TF_String* name, TF_Map* out_config, TF_Status* out_status);
        void (*set_config)(TF_PubSub_Channel* channel, const TF_String* name, const TF_Map* config, TF_Status* out_status);

        void (*get_stats)(TF_PubSub_Channel* channel, const TF_String* name, TF_Map* out_stats, TF_Status* out_status);
        void (*purge)(TF_PubSub_Channel* channel, const TF_String* name, TF_Status* out_status);

        void (*set_dead_letter)(TF_PubSub_Channel* channel, const TF_String* target_channel, const TF_String* dead_letter_channel, TF_Status* out_status);
        void (*list_dead_letters)(TF_PubSub_Channel* channel, const TF_String* target_channel, TF_Vector* out_payloads, TF_Status* out_status);
        void (*requeue_dead_letter)(TF_PubSub_Channel* channel, const TF_String* target_channel, const TF_String* payload, TF_Status* out_status);

        void (*list)(TF_PubSub_Channel* channel, TF_Vector* out_channels, TF_Status* out_status);
    } TF_PubSub_ChannelOps;

#define TF_PUBSUB_CHANNEL_STRUCT_SIZE TF_OFFSET_OF_END(TF_PubSub_ChannelOps, list)

    TF_CAPI_EXPORT void create_pubsub_channel(
        TF_PubSub_ChannelOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_channel(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_CHANNEL_H_

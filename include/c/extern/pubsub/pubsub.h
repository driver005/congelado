#ifndef TENSORFLOW_C_EXTERN_PUBSUB_H_
#define TENSORFLOW_C_EXTERN_PUBSUB_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include "c/extern/pubsub/subscription.h"
#include "c/extern/pubsub/channel.h"
#include "c/extern/pubsub/publish.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_PubSub
    {
        void* plugin_data;
        const TF_PubSub_SubscriptionOps* subscription_ops;
        const TF_PubSub_ChannelOps* channel_ops;
        const TF_PubSub_PublishOps* publish_ops;
    } TF_PubSub;

    typedef struct TF_PubSubOps
    {
        size_t struct_size;
        void (*destroy)(TF_PubSub* pubsub);
        void (*get_name)(TF_PubSub* pubsub, TF_String* out_name);
    } TF_PubSubOps;

#define TF_PUBSUB_STRUCT_SIZE TF_OFFSET_OF_END(TF_PubSubOps, get_name)

    TF_CAPI_EXPORT void create_pubsub(TF_PubSubOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_pubsub(void* plugin_context);

    static inline void init_pubsub(TF_PubSubOps** ops, TF_PubSub* pubsub, TF_Status* out_status)
    {
        create_pubsub(ops, &pubsub->plugin_data, out_status);

        TF_PubSub_SubscriptionOps* subscription_ops = NULL;
        create_pubsub_subscription(&subscription_ops, &pubsub->plugin_data, out_status);
        pubsub->subscription_ops = subscription_ops;

        TF_PubSub_ChannelOps* channel_ops = NULL;
        create_pubsub_channel(&channel_ops, &pubsub->plugin_data, out_status);
        pubsub->channel_ops = channel_ops;

        TF_PubSub_PublishOps* publish_ops = NULL;
        create_pubsub_publish(&publish_ops, &pubsub->plugin_data, out_status);
        pubsub->publish_ops = publish_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_PUBSUB_H_

#ifndef CONGELADO_C_PUBSUB_SUBSCRIPTION_H_
#define CONGELADO_C_PUBSUB_SUBSCRIPTION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TF_PubSub_Handler)(
        void* user_data,
        const TF_String* channel,
        const TF_String* payload,
        TF_PubSub_Subscription* subscription
    );

    typedef void (*TF_PubSub_IntFn)(void* user_data, int64_t value, TF_Status* out_status);

    typedef struct TF_PubSub_Subscription
    {
        void* plugin_data;
    } TF_PubSub_Subscription;

    typedef struct TF_PubSub_SubscriptionOps
    {
        size_t struct_size;
        void (*destroy)(TF_PubSub_Subscription* subscription);
        void (*unsubscribe)(TF_PubSub_Subscription* subscription);
        void (*ack)(TF_PubSub_Subscription* subscription, TF_Status* out_status);
        void (*nack)(TF_PubSub_Subscription* subscription, TF_Status* out_status);
        void (*seek)(TF_PubSub_Subscription* subscription, const TF_String* position, TF_Status* out_status);
        void (*get_lag)(TF_PubSub_Subscription* subscription, TF_PubSub_IntFn completion, void* user_data, TF_Status* out_status);
    } TF_PubSub_SubscriptionOps;

#define TF_PUBSUB_SUBSCRIPTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_PubSub_SubscriptionOps, get_lag)

    TF_CAPI_EXPORT void create_pubsub_subscription(
        TF_PubSub_SubscriptionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_subscription(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_SUBSCRIPTION_H_

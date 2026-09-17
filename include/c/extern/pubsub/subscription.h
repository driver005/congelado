#ifndef CONGELADO_C_PUBSUB_SUBSCRIPTION_H_
#define CONGELADO_C_PUBSUB_SUBSCRIPTION_H_

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFPubSubSubscription
    {
        void* plugin_data;
    } TFPubSubSubscription;

    typedef void (*TFPubSubHandler)(
        void* user_data,
        const TF_String* channel,
        const TF_String* payload,
        TFPubSubSubscription* subscription
    );

    typedef void (*TFPubSubIntFn)(void* user_data, int64_t value, TF_Status* out_status);

    typedef struct TFPubSubSubscriptionOps
    {
        size_t struct_size;
        void (*destroy)(TFPubSubSubscription* subscription);
        void (*unsubscribe)(TFPubSubSubscription* subscription);
        void (*ack)(TFPubSubSubscription* subscription, TF_Status* out_status);
        void (*nack)(TFPubSubSubscription* subscription, TF_Status* out_status);
        void (*seek)(
            TFPubSubSubscription* subscription,
            const TF_String* position,
            TF_Status* out_status
        );
        void (*get_lag)(
            TFPubSubSubscription* subscription,
            TFPubSubIntFn completion,
            void* user_data,
            TF_Status* out_status
        );
    } TFPubSubSubscriptionOps;

#define TF_PUBSUB_SUBSCRIPTION_STRUCT_SIZE TF_OFFSET_OF_END(TFPubSubSubscriptionOps, get_lag)

    TF_CAPI_EXPORT void create_pubsub_subscription(
        TFPubSubSubscriptionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_pubsub_subscription(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PUBSUB_SUBSCRIPTION_H_

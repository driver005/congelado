#ifndef TENSORFLOW_C_EXTERN_PUBSUB_H_
#define TENSORFLOW_C_EXTERN_PUBSUB_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_PubSub — generic message bus, replacing the separate events and logger domain vtables. Both are "publish a named message, optionally with subscribers" — logger's severity levels are just well-known channel names ("log.debug", "log.info", "log.important", "log.warning", "log.error", "log.fatal"), a documented convention, not a new enum.
    //
    // Brought to the real depth a production message bus needs (Kafka consumer groups, MQTT retained/QoS, RabbitMQ dead-lettering), which neither source domain individually exposed.
    typedef struct TF_PubSub_Subscription TF_PubSub_Subscription;

    // subscription is handed back so a handler can ack/nack the delivery it is currently processing.
    typedef void (*TF_PubSub_Handler)(
        void* user_data,
        const TF_String* channel,
        const TF_String* payload,
        TF_PubSub_Subscription* subscription
    );

    typedef void (*TF_PubSub_RetainedFn)(void* user_data, const TF_String* payload, TF_Status* status);
    typedef void (*TF_PubSub_AckFn)(void* user_data, TF_Status* status);
    typedef void (*TF_PubSub_IntFn)(void* user_data, int64_t value, TF_Status* status);

    // Plugin-facing vtable registered via create_pubsub.
    typedef struct TF_PubSubOps
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        int (*is_connected)(void* plugin_context);

        // retain != 0: the backend keeps this as the channel's last-known value, handed to any future subscriber immediately on subscribe (and retrievable via get_retained) — an MQTT-style retained message.
        void (*publish)(void* plugin_context, const TF_String* channel, const TF_String* payload, int retain, TF_Status* status);

        void (*publish_batch)(
            void* plugin_context,
            const TF_String* channel,
            const TF_Vector* payloads,
            TF_PubSub_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        // Block until every publish so far on this plugin_context has been delivered/persisted by the backend — a durability guarantee.
        void (*flush)(void* plugin_context, TF_PubSub_AckFn completion, void* user_data, TF_Status* status);

        // pattern: exact channel name or a glob-style wildcard (e.g. "log.*", "order.*.created") — documented convention; wildcard support is backend-dependent, a plugin that can't match patterns treats pattern as an exact channel name.
        TF_PubSub_Subscription* (*subscribe)(
            void* plugin_context,
            const TF_String* pattern,
            TF_PubSub_Handler handler,
            void* user_data,
            TF_Status* status
        );

        // Kafka-style consumer-group subscription: messages load-balance across every subscriber sharing the same group_id rather than fanning out to all.
        TF_PubSub_Subscription* (*subscribe_group)(
            void* plugin_context,
            const TF_String* pattern,
            const TF_String* group_id,
            TF_PubSub_Handler handler,
            void* user_data,
            TF_Status* status
        );

        // Auto-unsubscribes after its first delivered message — a one-shot request/reply-style convenience.
        TF_PubSub_Subscription* (*subscribe_once)(
            void* plugin_context,
            const TF_String* pattern,
            TF_PubSub_Handler handler,
            void* user_data,
            TF_Status* status
        );

        void (*unsubscribe)(TF_PubSub_Subscription* subscription);

        // At-least-once delivery: ack marks a message processed, nack requeues it (optionally to the channel's dead-letter target after enough nacks).
        void (*ack)(TF_PubSub_Subscription* subscription, TF_Status* status);
        void (*nack)(TF_PubSub_Subscription* subscription, TF_Status* status);

        // Reposition a subscription's read cursor for replay (by offset or, if the backend supports it, a timestamp encoded in the TF_String).
        void (*seek)(TF_PubSub_Subscription* subscription, const TF_String* position, TF_Status* status);
        void (*get_subscription_lag)(TF_PubSub_Subscription* subscription, TF_PubSub_IntFn completion, void* user_data, TF_Status* status);

        void (*get_retained)(void* plugin_context, const TF_String* channel, TF_PubSub_RetainedFn completion, void* user_data, TF_Status* status);

        // Channel/topic lifecycle and configuration.
        void (*create_channel)(void* plugin_context, const TF_String* name, const TF_Map* config, TF_Status* status);
        void (*delete_channel)(void* plugin_context, const TF_String* name, TF_Status* status);
        void (*get_channel_config)(void* plugin_context, const TF_String* name, TF_Map* out_config, TF_Status* status);
        void (*set_channel_config)(void* plugin_context, const TF_String* name, const TF_Map* config, TF_Status* status);

        // out_stats keys such as message_count/size_bytes/subscriber_count are a documented convention, not enforced by this header.
        void (*get_channel_stats)(void* plugin_context, const TF_String* name, TF_Map* out_stats, TF_Status* status);
        void (*purge_channel)(void* plugin_context, const TF_String* name, TF_Status* status);

        // Dead-letter handling for messages that repeatedly fail delivery.
        void (*set_dead_letter_channel)(void* plugin_context, const TF_String* channel, const TF_String* dead_letter_channel, TF_Status* status);
        void (*list_dead_letters)(void* plugin_context, const TF_String* channel, TF_Vector* out_payloads, TF_Status* status);
        void (*requeue_dead_letter)(void* plugin_context, const TF_String* channel, const TF_String* payload, TF_Status* status);

        void (*list_channels)(void* plugin_context, TF_Vector* out_channels, TF_Status* status);
        void (*list_subscriptions)(void* plugin_context, TF_Vector* out_patterns, TF_Status* status);

    } TF_PubSubOps;

#define TF_PUBSUB_STRUCT_SIZE TF_OFFSET_OF_END(TF_PubSubOps, list_subscriptions)

    TF_CAPI_EXPORT void create_pubsub(TF_PubSubOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_pubsub(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_PUBSUB_H_

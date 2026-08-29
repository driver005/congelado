module;

#include "c/extern/otel/enums.h"

export module cc_abi_builder_otel:enums;

export namespace ice::builder {

enum class SpanKind
{
    Internal = TF_OTEL_INTERNAL,
    Server = TF_OTEL_SERVER,
    Client = TF_OTEL_CLIENT,
    Producer = TF_OTEL_PRODUCER,
    Consumer = TF_OTEL_CONSUMER,
};

enum class SpanStatus
{
    Unset = TF_OTEL_UNSET,
    Ok = TF_OTEL_OK,
    Error = TF_OTEL_ERROR,
};

} // namespace ice::builder

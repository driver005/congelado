// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/hash/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/hash/hash.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_hash;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Hash
{
public:
    static Hash* create(void* ctx) noexcept
    {
        return static_cast<Hash*>(ctx);
    }

    template<typename HandleT>
    static Hash* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Hash*>(handle);
    }

    virtual ~Hash() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    hash_bytes(const void* data, size_t size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    hash_combine(size_t seed, size_t value) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Hash* get_generic_vtable()
    {
        static TF_Hash vtable = {
            .struct_size = TF_HASH_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Hash::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .hash_bytes =
                [](void* plugin_context, const void* data, size_t size) noexcept
            {
                auto* self = Hash::create(plugin_context);
                auto res = self->hash_bytes(data, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .hash_combine =
                [](void* plugin_context, size_t seed, size_t value) noexcept
            {
                auto* self = Hash::create(plugin_context);
                auto res = self->hash_combine(seed, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"

export module cc_abi_builder_store;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFStoreQueryOps
{
public:
    static TFStoreQueryOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreQueryOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreQueryOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreQueryOps*>(handle->plugin_data);
    }

    virtual ~TFStoreQueryOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    run(const ice::sonic::TF_MapOps& filters,
        const ice::sonic::TF_StringOps& free_text,
        const ice::sonic::TF_StringOps& sort,
        size_t offset,
        size_t limit,
        TFStoreQueryFn completion,
        void* user_data) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFStoreQueryOps* get_generic_vtable()
    {
        static TFStoreQueryOps vtable = {
            .struct_size = TF_TOREQUERY_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFStoreQueryOps::create(plugin_context);
            },
            .run =
                [](TFStoreQuery* query,
                   const TF_Map* filters,
                   const TF_String* free_text,
                   const TF_String* sort,
                   size_t offset,
                   size_t limit,
                   TFStoreQueryFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFStoreQueryOps::create(query);
                auto res = self->run(
                    ice::sonic::TF_MapOps::wrap(filters),
                    ice::sonic::TF_StringOps::wrap(free_text),
                    ice::sonic::TF_StringOps::wrap(sort),
                    offset,
                    limit,
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

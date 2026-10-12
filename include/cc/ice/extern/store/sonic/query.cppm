// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_store_sonic:query;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class StoreQuery : public ice::sonic::Runtime<::TFStoreQueryOps, ::TFStoreQuery>
{
public:
    StoreQuery(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    StoreQuery(
        const ice::sonic::Registration& registry,
        ::TFStoreQuery* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit StoreQuery(const ::TFStoreQueryOps* ops) noexcept :
        Runtime(ops)
    {
    }

    StoreQuery(const ::TFStoreQueryOps* ops, ::TFStoreQuery* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void run(
        const ice::sonic::Map& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TFStoreQueryFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->run(
            get_handle(),
            filters.get_handle(),
            free_text.get_handle(),
            sort.get_handle(),
            offset,
            limit,
            completion,
            user_data,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic

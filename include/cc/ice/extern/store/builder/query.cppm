// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"

export module cc_ice_extern_store_builder:query;

import std;

export namespace ice::builder {

class TFStoreQueryOps
{
public:
    TFStoreQueryOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFStoreQueryOps(const TFStoreQueryOps&) = delete;
    TFStoreQueryOps& operator=(const TFStoreQueryOps&) = delete;

    static TFStoreQueryOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreQueryOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreQueryOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreQueryOps*>(handle->plugin_data);
    }

    virtual ~TFStoreQueryOps() = default;
    virtual void destroy() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    run(const ice::sonic::TF_MapOps& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TFStoreQueryFn completion,
        void* user_data) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreQueryOps{
            .struct_size = TF_TOREQUERY_STRUCT_SIZE,
            .destroy =
                [](TFStoreQuery* query) noexcept
            {
                TFStoreQueryOps::from_handle(query).destroy();
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
                auto res = TFStoreQueryOps::from_handle(query).run(
                    ice::sonic::TF_MapOps::wrap(filters),
                    ice::sonic::String::wrap(free_text),
                    ice::sonic::String::wrap(sort),
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
    }

    const ::TFStoreQueryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreQuery& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreQueryOps m_vtable;
    TFStoreQuery m_handle;
};

} // namespace ice::builder

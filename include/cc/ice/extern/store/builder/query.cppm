// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_store_builder:query;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class StoreQuery
{
public:
    explicit StoreQuery(
        const ::TF_MapOps* Map_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Map_ops = Map_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    StoreQuery(const StoreQuery&) = delete;
    StoreQuery& operator=(const StoreQuery&) = delete;

    static StoreQuery& from_handle(void* ctx) noexcept
    {
        return *static_cast<StoreQuery*>(ctx);
    }

    template<typename HandleT>
    static StoreQuery& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<StoreQuery*>(handle->plugin_data);
    }

    virtual ~StoreQuery() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    run(const ice::sonic::Map& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TFStoreQueryFn completion,
        void* user_data,
        const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreQuery*)) noexcept
    {
        m_vtable = ::TFStoreQueryOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreQueryOps, run),

            .create = create,
            .destroy =
                [](::TFStoreQuery* handle) noexcept
            {
                auto& self = StoreQuery::from_handle(handle);
                self.destroy();
            },
            .run =
                [](::TFStoreQuery* query,
                   const ::TF_Map* filters,
                   const ::TF_String* free_text,
                   const ::TF_String* sort,
                   size_t offset,
                   size_t limit,
                   TFStoreQueryFn completion,
                   void* user_data,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = StoreQuery::from_handle(query);
                self.run(
                    self.wrap(std::type_identity<ice::sonic::Map>{}, filters),
                    self.wrap(std::type_identity<ice::sonic::String>{}, free_text),
                    self.wrap(std::type_identity<ice::sonic::String>{}, sort),
                    offset,
                    limit,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Map wrap(std::type_identity<ice::sonic::Map>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::Map{m_Map_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFStoreQueryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreQuery& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreQueryOps*>(&m_vtable));
    }

private:
    ::TFStoreQueryOps m_vtable;
    ::TFStoreQuery m_handle;

    const ::TF_MapOps* m_Map_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

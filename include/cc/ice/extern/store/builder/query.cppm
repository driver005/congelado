// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_store_builder:query;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFStoreQueryOps
{
public:
    explicit TFStoreQueryOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void
    run(const ice::sonic::TF_MapOps& filters,
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
                [](TFStoreQuery* handle) noexcept
            {
                auto& self = TFStoreQueryOps::from_handle(handle);
                self.destroy();
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
                auto& self = TFStoreQueryOps::from_handle(query);
                self.run(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, filters),
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

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFStoreQueryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreQuery& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreQueryOps*>(&m_vtable));
    }

private:
    ::TFStoreQueryOps m_vtable;
    ::TFStoreQuery m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

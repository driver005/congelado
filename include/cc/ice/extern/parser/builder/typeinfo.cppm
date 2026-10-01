// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/typeinfo.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_parser_builder:typeinfo;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFParserTypeInfoOps
{
public:
    explicit TFParserTypeInfoOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TFParserTypeInfoOps(const TFParserTypeInfoOps&) = delete;
    TFParserTypeInfoOps& operator=(const TFParserTypeInfoOps&) = delete;

    static TFParserTypeInfoOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserTypeInfoOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserTypeInfoOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserTypeInfoOps*>(handle->plugin_data);
    }

    virtual ~TFParserTypeInfoOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_dtype(int* out_dtype, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_shape(
        int64_t** out_dims,
        int* out_num_dims,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserTypeInfo*)) noexcept
    {
        m_vtable = ::TFParserTypeInfoOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserTypeInfoOps, get_shape),

            .create = create,
            .destroy =
                [](TFParserTypeInfo* handle) noexcept
            {
                auto& self = TFParserTypeInfoOps::from_handle(handle);
                self.destroy();
            },
            .get_dtype =
                [](TFParserTypeInfo* typeinfo, int* out_dtype, TF_Status* out_status) noexcept
            {
                auto& self = TFParserTypeInfoOps::from_handle(typeinfo);
                self.get_dtype(
                    out_dtype,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_shape =
                [](TFParserTypeInfo* typeinfo,
                   int64_t** out_dims,
                   int* out_num_dims,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFParserTypeInfoOps::from_handle(typeinfo);
                self.get_shape(
                    out_dims,
                    out_num_dims,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TFParserTypeInfoOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserTypeInfo& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFParserTypeInfoOps*>(&m_vtable));
    }

private:
    ::TFParserTypeInfoOps m_vtable;
    ::TFParserTypeInfo m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

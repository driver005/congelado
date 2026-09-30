// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/attrtype.h"

export module cc_ice_intern_builder:attrtype;

import std;

export namespace ice::builder {

class TF_AttrTypeOps
{
public:
    TF_AttrTypeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_AttrTypeOps(const TF_AttrTypeOps&) = delete;
    TF_AttrTypeOps& operator=(const TF_AttrTypeOps&) = delete;

    static TF_AttrTypeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_AttrTypeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_AttrTypeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_AttrTypeOps*>(handle->plugin_data);
    }

    virtual ~TF_AttrTypeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    attrtype_name(TFAttrTypeEnum type, const ice::sonic::String& out_type_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_AttrTypeOps{
            .struct_size = TF_ATTRTYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_AttrTypeOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .attrtype_name =
                [](TF_AttrType* attrtype, TFAttrTypeEnum type, TF_String* out_type_name) noexcept
            {
                auto res = TF_AttrTypeOps::from_handle(attrtype).attrtype_name(
                    type,
                    ice::sonic::String::wrap(out_type_name)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_AttrTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_AttrType& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_AttrTypeOps m_vtable;
    TF_AttrType m_handle;
};

} // namespace ice::builder

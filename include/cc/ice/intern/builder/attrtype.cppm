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
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void
    attrtype_name(TFAttrTypeEnum type, const ice::sonic::String& out_type_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_AttrTypeOps{
            .struct_size = TF_ATTRTYPE_STRUCT_SIZE,
            .get_name =
                [](TF_AttrType* attrtype, TF_String* out_name) noexcept
            {
                TF_AttrTypeOps::from_handle(attrtype).get_name(ice::sonic::String::wrap(out_name));
            },
            .attrtype_name =
                [](TF_AttrType* attrtype, TFAttrTypeEnum type, TF_String* out_type_name) noexcept
            {
                TF_AttrTypeOps::from_handle(attrtype).attrtype_name(
                    type,
                    ice::sonic::String::wrap(out_type_name)
                );
            },

        };
    }

    const ::TF_AttrTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_AttrType& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_AttrTypeOps m_vtable;
    TF_AttrType m_handle;
};

} // namespace ice::builder

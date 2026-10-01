// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/attrtype.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:attrtype;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_AttrTypeOps
{
public:
    explicit TF_AttrTypeOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void
    attrtype_name(TFAttrTypeEnum type, const ice::sonic::String& out_type_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_AttrType*)) noexcept
    {
        m_vtable = ::TF_AttrTypeOps{
            .struct_size = TF_OFFSET_OF_END(::TF_AttrTypeOps, attrtype_name),

            .create = create,
            .destroy =
                [](TF_AttrType* handle) noexcept
            {
                auto& self = TF_AttrTypeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_AttrType* attrtype, TF_String* out_name) noexcept
            {
                auto& self = TF_AttrTypeOps::from_handle(attrtype);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .attrtype_name =
                [](TF_AttrType* attrtype, TFAttrTypeEnum type, TF_String* out_type_name) noexcept
            {
                auto& self = TF_AttrTypeOps::from_handle(attrtype);
                self.attrtype_name(
                    type,
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_type_name)
                );
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_AttrTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_AttrType& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_AttrTypeOps*>(&m_vtable));
    }

private:
    ::TF_AttrTypeOps m_vtable;
    ::TF_AttrType m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

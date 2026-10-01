// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:tstring;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class String
{
public:
    explicit String() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    String(const String&) = delete;
    String& operator=(const String&) = delete;

    static String& from_handle(void* ctx) noexcept
    {
        return *static_cast<String*>(ctx);
    }

    template<typename HandleT>
    static String& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<String*>(handle->plugin_data);
    }

    virtual ~String() = default;
    virtual void destroy() noexcept = 0;
    virtual void init() noexcept = 0;
    virtual void copy(const char* src, size_t size) noexcept = 0;
    virtual void assign_view(const char* src, size_t size) noexcept = 0;
    virtual void get_data_pointer(const char** out_data) noexcept = 0;
    virtual void get_type(TFTStringType* out_type) noexcept = 0;
    virtual void get_size(size_t* out_size) noexcept = 0;
    virtual void get_capacity(size_t* out_capacity) noexcept = 0;
    virtual void dealloc() noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_String*)) noexcept
    {
        m_vtable = ::TF_StringOps{
            .struct_size = TF_OFFSET_OF_END(::TF_StringOps, dealloc),

            .create = create,
            .destroy =
                [](TF_String* handle) noexcept
            {
                auto& self = String::from_handle(handle);
                self.destroy();
            },
            .init =
                [](TF_String* t) noexcept
            {
                auto& self = String::from_handle(t);
                self.init();
            },
            .copy =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto& self = String::from_handle(dst);
                self.copy(src, size);
            },
            .assign_view =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto& self = String::from_handle(dst);
                self.assign_view(src, size);
            },
            .get_data_pointer =
                [](const TF_String* t, const char** out_data) noexcept
            {
                auto& self = String::from_handle(t);
                self.get_data_pointer(out_data);
            },
            .get_type =
                [](const TF_String* t, TFTStringType* out_type) noexcept
            {
                auto& self = String::from_handle(t);
                self.get_type(out_type);
            },
            .get_size =
                [](const TF_String* t, size_t* out_size) noexcept
            {
                auto& self = String::from_handle(t);
                self.get_size(out_size);
            },
            .get_capacity =
                [](const TF_String* t, size_t* out_capacity) noexcept
            {
                auto& self = String::from_handle(t);
                self.get_capacity(out_capacity);
            },
            .dealloc =
                [](TF_String* t) noexcept
            {
                auto& self = String::from_handle(t);
                self.dealloc();
            },

        };
    }

    const ::TF_StringOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_String& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_StringOps*>(&m_vtable));
    }

private:
    ::TF_StringOps m_vtable;
    ::TF_String m_handle;
};

} // namespace ice::builder

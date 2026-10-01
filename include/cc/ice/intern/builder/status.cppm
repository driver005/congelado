// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:status;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Status
{
public:
    explicit Status(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;

    static Status& from_handle(void* ctx) noexcept
    {
        return *static_cast<Status*>(ctx);
    }

    template<typename HandleT>
    static Status& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Status*>(handle->plugin_data);
    }

    virtual ~Status() = default;
    virtual void destroy() noexcept = 0;
    virtual void delete_status() noexcept = 0;
    virtual void set_status(TF_Code code, const ice::sonic::String& msg) noexcept = 0;
    virtual void
    set_payload(const ice::sonic::String& key, const ice::sonic::String& value) noexcept = 0;
    virtual void for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept = 0;
    virtual void
    set_status_from_io_error(int error_code, const ice::sonic::String& context) noexcept = 0;
    virtual void get_code(TF_Code* out_code) noexcept = 0;
    virtual void message(const ice::sonic::String& out_message) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Status*)) noexcept
    {
        m_vtable = ::TF_StatusOps{
            .struct_size = TF_OFFSET_OF_END(::TF_StatusOps, message),

            .create = create,
            .destroy =
                [](TF_Status* handle) noexcept
            {
                auto& self = Status::from_handle(handle);
                self.destroy();
            },
            .delete_status =
                [](TF_Status* s) noexcept
            {
                auto& self = Status::from_handle(s);
                self.delete_status();
            },
            .set_status =
                [](TF_Status* s, TF_Code code, const TF_String* msg) noexcept
            {
                auto& self = Status::from_handle(s);
                self.set_status(code, self.wrap(std::type_identity<ice::sonic::String>{}, msg));
            },
            .set_payload =
                [](TF_Status* s, const TF_String* key, const TF_String* value) noexcept
            {
                auto& self = Status::from_handle(s);
                self.set_payload(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value)
                );
            },
            .for_each_payload =
                [](const TF_Status* s, TF_PayloadVisitor visitor, void* capture) noexcept
            {
                auto& self = Status::from_handle(s);
                self.for_each_payload(visitor, capture);
            },
            .set_status_from_io_error =
                [](TF_Status* s, int error_code, const TF_String* context) noexcept
            {
                auto& self = Status::from_handle(s);
                self.set_status_from_io_error(
                    error_code,
                    self.wrap(std::type_identity<ice::sonic::String>{}, context)
                );
            },
            .get_code =
                [](const TF_Status* s, TF_Code* out_code) noexcept
            {
                auto& self = Status::from_handle(s);
                self.get_code(out_code);
            },
            .message =
                [](const TF_Status* s, TF_String* out_message) noexcept
            {
                auto& self = Status::from_handle(s);
                self.message(self.wrap(std::type_identity<ice::sonic::String>{}, out_message));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_StatusOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Status& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_StatusOps*>(&m_vtable));
    }

private:
    ::TF_StatusOps m_vtable;
    ::TF_Status m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

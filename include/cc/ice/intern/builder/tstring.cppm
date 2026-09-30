// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:tstring;

import std;

export namespace ice::builder {

class String
{
public:
    String() noexcept :
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
    [[nodiscard]] virtual std::expected<void, ice::Status> init() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    copy(const char* src, size_t size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    assign_view(const char* src, size_t size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_data_pointer(const char** out_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_type(TFTStringType* out_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_capacity(size_t* out_capacity) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> dealloc() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StringOps{
            .struct_size = TF_STRING_STRUCT_SIZE,
            .init =
                [](TF_String* t) noexcept
            {
                auto res = String::from_handle(t).init();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .copy =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto res = String::from_handle(dst).copy(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .assign_view =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto res = String::from_handle(dst).assign_view(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_data_pointer =
                [](const TF_String* t, const char** out_data) noexcept
            {
                auto res = String::from_handle(t).get_data_pointer(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type =
                [](const TF_String* t, TFTStringType* out_type) noexcept
            {
                auto res = String::from_handle(t).get_type(out_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_size =
                [](const TF_String* t, size_t* out_size) noexcept
            {
                auto res = String::from_handle(t).get_size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_capacity =
                [](const TF_String* t, size_t* out_capacity) noexcept
            {
                auto res = String::from_handle(t).get_capacity(out_capacity);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dealloc =
                [](TF_String* t) noexcept
            {
                auto res = String::from_handle(t).dealloc();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_StringOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_String& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_StringOps m_vtable;
    TF_String m_handle;
};

} // namespace ice::builder

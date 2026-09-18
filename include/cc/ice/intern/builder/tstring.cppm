// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_ice_builder_intern:tstring;

import std;

export namespace ice::builder {

class TF_StringOps
{
public:
    static TF_StringOps* create(void* ctx) noexcept
    {
        return static_cast<TF_StringOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StringOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_StringOps*>(handle->plugin_data);
    }

    virtual ~TF_StringOps() = default;
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

    static TF_StringOps* get_generic_vtable()
    {
        static TF_StringOps vtable = {
            .struct_size = TF_STRING_STRUCT_SIZE,
            .init =
                [](TF_String* t) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->init();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .copy =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto* self = TF_StringOps::create(dst);
                auto res = self->copy(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .assign_view =
                [](TF_String* dst, const char* src, size_t size) noexcept
            {
                auto* self = TF_StringOps::create(dst);
                auto res = self->assign_view(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_data_pointer =
                [](const TF_String* t, const char** out_data) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->get_data_pointer(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type =
                [](const TF_String* t, TFTStringType* out_type) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->get_type(out_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_size =
                [](const TF_String* t, size_t* out_size) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->get_size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_capacity =
                [](const TF_String* t, size_t* out_capacity) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->get_capacity(out_capacity);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dealloc =
                [](TF_String* t) noexcept
            {
                auto* self = TF_StringOps::create(t);
                auto res = self->dealloc();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder

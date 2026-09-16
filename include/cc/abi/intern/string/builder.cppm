// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/string/string.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/string/string.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_string;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class String
{
public:
    static String* create(void* ctx) noexcept
    {
        return static_cast<String*>(ctx);
    }

    template<typename HandleT>
    static String* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<String*>(handle);
    }

    virtual ~String() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_tstring() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> init() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> copy(const char* src, size_t size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    assign_view(const char* src, size_t size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_data_pointer() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_type() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_capacity() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> dealloc() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_String* get_generic_vtable()
    {
        static TF_String vtable = {
            .struct_size = TF_STRING_STRUCT_SIZE,
            .new_tstring =
                [](void* plugin_context) noexcept
            {
                auto* self = String::create(plugin_context);
                auto res = self->new_tstring();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .init =
                [](TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->init();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .copy =
                [](TF_String_Handle* dst, const char* src, size_t size) noexcept
            {
                auto* self = String::create(dst);
                auto res = self->copy(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .assign_view =
                [](TF_String_Handle* dst, const char* src, size_t size) noexcept
            {
                auto* self = String::create(dst);
                auto res = self->assign_view(src, size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_data_pointer =
                [](const TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->get_data_pointer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type =
                [](const TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->get_type();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_size =
                [](const TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->get_size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_capacity =
                [](const TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->get_capacity();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dealloc =
                [](TF_String_Handle* t) noexcept
            {
                auto* self = String::create(t);
                auto res = self->dealloc();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

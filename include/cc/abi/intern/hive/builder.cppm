// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/hive/hive.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_hive;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Hive
{
public:
    static Hive* create(void* ctx) noexcept
    {
        return static_cast<Hive*>(ctx);
    }

    template<typename HandleT>
    static Hive* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Hive*>(handle);
    }

    virtual ~Hive() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_hive(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> insert(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> erase(TF_Hive_Slot* slot) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get(const TF_Hive_Slot* slot) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_HiveVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Hive* get_generic_vtable()
    {
        static TF_Hive vtable = {
            .struct_size = TF_HIVE_STRUCT_SIZE,
            .new_hive =
                [](void* plugin_context, size_t element_size) noexcept
            {
                auto* self = Hive::create(plugin_context);
                auto res = self->new_hive(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .insert =
                [](TF_Hive_Handle* hive, const void* value) noexcept
            {
                auto* self = Hive::create(hive);
                auto res = self->insert(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .erase =
                [](TF_Hive_Handle* hive, TF_Hive_Slot* slot) noexcept
            {
                auto* self = Hive::create(hive);
                auto res = self->erase(slot);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Hive_Handle* hive, const TF_Hive_Slot* slot) noexcept
            {
                auto* self = Hive::create(hive);
                auto res = self->get(slot);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each =
                [](const TF_Hive_Handle* hive, TF_HiveVisitor visitor, void* capture) noexcept
            {
                auto* self = Hive::create(hive);
                auto res = self->for_each(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Hive_Handle* hive) noexcept
            {
                auto* self = Hive::create(hive);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Hive::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

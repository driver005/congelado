// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_FileStatisticsOps
{
public:
    static TF_FileStatisticsOps* create(void* ctx) noexcept
    {
        return static_cast<TF_FileStatisticsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_FileStatisticsOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_FileStatisticsOps*>(handle->plugin_data);
    }

    virtual ~TF_FileStatisticsOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> is_directory(int* out_is_directory) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_is_directory(int is_directory) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> length(int64_t* out_length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_length(int64_t length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> mtime_nsec(int64_t* out_mtime_nsec) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_mtime_nsec(int64_t mtime_nsec) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_FileStatisticsOps* get_generic_vtable()
    {
        static TF_FileStatisticsOps vtable = {
            .struct_size = TF_FILESTATISTICS_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .is_directory =
                [](const TF_FileStatistics* stats, int* out_is_directory) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->is_directory(out_is_directory);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_directory =
                [](TF_FileStatistics* stats, int is_directory) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->set_is_directory(is_directory);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .length =
                [](const TF_FileStatistics* stats, int64_t* out_length) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->length(out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_length =
                [](TF_FileStatistics* stats, int64_t length) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->set_length(length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .mtime_nsec =
                [](const TF_FileStatistics* stats, int64_t* out_mtime_nsec) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->mtime_nsec(out_mtime_nsec);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_mtime_nsec =
                [](TF_FileStatistics* stats, int64_t mtime_nsec) noexcept
            {
                auto* self = TF_FileStatisticsOps::create(stats);
                auto res = self->set_mtime_nsec(mtime_nsec);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_FileStatisticsOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder

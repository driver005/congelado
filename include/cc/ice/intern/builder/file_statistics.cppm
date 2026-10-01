// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/file_statistics.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/file_statistics.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:file_statistics;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_FileStatisticsOps
{
public:
    explicit TF_FileStatisticsOps(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    TF_FileStatisticsOps(const TF_FileStatisticsOps&) = delete;
    TF_FileStatisticsOps& operator=(const TF_FileStatisticsOps&) = delete;

    static TF_FileStatisticsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_FileStatisticsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_FileStatisticsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_FileStatisticsOps*>(handle->plugin_data);
    }

    virtual ~TF_FileStatisticsOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void is_directory(int* out_is_directory) noexcept = 0;
    virtual void set_is_directory(int is_directory) noexcept = 0;
    virtual void length(int64_t* out_length) noexcept = 0;
    virtual void set_length(int64_t length) noexcept = 0;
    virtual void mtime_nsec(int64_t* out_mtime_nsec) noexcept = 0;
    virtual void set_mtime_nsec(int64_t mtime_nsec) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_FileStatistics*)) noexcept
    {
        m_vtable = ::TF_FileStatisticsOps{
            .struct_size = TF_OFFSET_OF_END(::TF_FileStatisticsOps, set_mtime_nsec),

            .create = create,
            .destroy =
                [](TF_FileStatistics* handle) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_FileStatistics* stats, TF_String* out_name) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .is_directory =
                [](const TF_FileStatistics* stats, int* out_is_directory) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.is_directory(out_is_directory);
            },
            .set_is_directory =
                [](TF_FileStatistics* stats, int is_directory) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.set_is_directory(is_directory);
            },
            .length =
                [](const TF_FileStatistics* stats, int64_t* out_length) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.length(out_length);
            },
            .set_length =
                [](TF_FileStatistics* stats, int64_t length) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.set_length(length);
            },
            .mtime_nsec =
                [](const TF_FileStatistics* stats, int64_t* out_mtime_nsec) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.mtime_nsec(out_mtime_nsec);
            },
            .set_mtime_nsec =
                [](TF_FileStatistics* stats, int64_t mtime_nsec) noexcept
            {
                auto& self = TF_FileStatisticsOps::from_handle(stats);
                self.set_mtime_nsec(mtime_nsec);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_FileStatisticsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_FileStatistics& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_FileStatisticsOps*>(&m_vtable));
    }

private:
    ::TF_FileStatisticsOps m_vtable;
    ::TF_FileStatistics m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

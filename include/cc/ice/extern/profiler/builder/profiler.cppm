// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/profiler/profiler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_profiler_builder:profiler;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ProfilerOps
{
public:
    explicit TF_ProfilerOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_ProfilerOps(const TF_ProfilerOps&) = delete;
    TF_ProfilerOps& operator=(const TF_ProfilerOps&) = delete;

    static TF_ProfilerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ProfilerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ProfilerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ProfilerOps*>(handle->plugin_data);
    }

    virtual ~TF_ProfilerOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_device_type(const ice::sonic::String& out_device_type) noexcept = 0;
    virtual void start(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void stop(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    collect_data_xspace(TF_Tensor** out_data, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Profiler*)) noexcept
    {
        m_vtable = ::TF_ProfilerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ProfilerOps, collect_data_xspace),

            .create = create,
            .destroy =
                [](TF_Profiler* handle) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Profiler* profiler, TF_String* out_name) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(profiler);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .get_device_type =
                [](TF_Profiler* profiler, TF_String* out_device_type) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(profiler);
                self.get_device_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_device_type)
                );
            },
            .start =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(profiler);
                self.start(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .stop =
                [](TF_Profiler* profiler, TF_Status* out_status) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(profiler);
                self.stop(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .collect_data_xspace =
                [](TF_Profiler* profiler, TF_Tensor** out_data, TF_Status* out_status) noexcept
            {
                auto& self = TF_ProfilerOps::from_handle(profiler);
                self.collect_data_xspace(
                    out_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_ProfilerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Profiler& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_ProfilerOps*>(&m_vtable));
    }

private:
    ::TF_ProfilerOps m_vtable;
    ::TF_Profiler m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder

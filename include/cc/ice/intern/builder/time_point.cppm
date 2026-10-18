// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/duration.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/time_point.h"

export module cc_ice_intern_builder:time_point;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TimePoint
{
public:
    explicit TimePoint(const ::TF_DurationOps* Duration_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Duration_ops = Duration_ops;
    }

    TimePoint(const TimePoint&) = delete;
    TimePoint& operator=(const TimePoint&) = delete;

    static TimePoint& from_handle(void* ctx) noexcept
    {
        return *static_cast<TimePoint*>(ctx);
    }

    template<typename HandleT>
    static TimePoint& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TimePoint*>(handle->plugin_data);
    }

    virtual ~TimePoint() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_duration_since_epoch(const ice::sonic::Duration& out_duration) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_TimePoint*)) noexcept
    {
        m_vtable = ::TF_TimePointOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TimePointOps, get_duration_since_epoch),

            .create = create,
            .destroy =
                [](::TF_TimePoint* handle) noexcept
            {
                auto& self = TimePoint::from_handle(handle);
                self.destroy();
            },
            .get_duration_since_epoch =
                [](const ::TF_TimePoint* time_point, ::TF_Duration* out_duration) noexcept
            {
                auto& self = TimePoint::from_handle(time_point);
                self.get_duration_since_epoch(
                    self.wrap(std::type_identity<ice::sonic::Duration>{}, out_duration)
                );
            },

        };
    }

    ice::sonic::Duration
    wrap(std::type_identity<ice::sonic::Duration>, const ::TF_Duration* handle) const noexcept
    {
        return ice::sonic::Duration{m_Duration_ops, handle};
    }

    const ::TF_TimePointOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_TimePoint& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_TimePointOps*>(&m_vtable));
    }

private:
    ::TF_TimePointOps m_vtable;
    ::TF_TimePoint m_handle;

    const ::TF_DurationOps* m_Duration_ops{nullptr};
};

} // namespace ice::builder

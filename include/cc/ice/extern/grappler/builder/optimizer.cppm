// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/optimizer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_grappler_builder:optimizer;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerOptimizerOps
{
public:
    explicit TFGrapplerOptimizerOps(
        const ::TFGrapplerItemOps* TFGrapplerItemOps_ops,
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGrapplerItemOps_ops = TFGrapplerItemOps_ops;
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_Status_ops = Status_ops;
    }

    TFGrapplerOptimizerOps(const TFGrapplerOptimizerOps&) = delete;
    TFGrapplerOptimizerOps& operator=(const TFGrapplerOptimizerOps&) = delete;

    static TFGrapplerOptimizerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerOptimizerOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerOptimizerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerOptimizerOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerOptimizerOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void optimize(
        const ice::sonic::TF_BufferOps& graph_buf,
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::TF_BufferOps& out_optimized_graph_buf,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerOptimizer*)) noexcept
    {
        m_vtable = ::TFGrapplerOptimizerOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerOptimizerOps, optimize),

            .create = create,
            .destroy =
                [](TFGrapplerOptimizer* handle) noexcept
            {
                auto& self = TFGrapplerOptimizerOps::from_handle(handle);
                self.destroy();
            },
            .optimize =
                [](TFGrapplerOptimizer* optimizer,
                   const TF_Buffer* graph_buf,
                   const TFGrapplerItem* item,
                   TF_Buffer* out_optimized_graph_buf,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerOptimizerOps::from_handle(optimizer);
                self.optimize(
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, graph_buf),
                    self.wrap(std::type_identity<ice::sonic::TFGrapplerItemOps>{}, item),
                    self.wrap(
                        std::type_identity<ice::sonic::TF_BufferOps>{},
                        out_optimized_graph_buf
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFGrapplerItemOps wrap(
        std::type_identity<ice::sonic::TFGrapplerItemOps>,
        const ::TFGrapplerItem* handle
    ) const noexcept
    {
        return ice::sonic::TFGrapplerItemOps{
            m_TFGrapplerItemOps_ops,
            const_cast<::TFGrapplerItem*>(handle)
        };
    }

    ice::sonic::TF_BufferOps
    wrap(std::type_identity<ice::sonic::TF_BufferOps>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::TF_BufferOps{m_TF_BufferOps_ops, const_cast<::TF_Buffer*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TFGrapplerOptimizerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerOptimizer& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFGrapplerOptimizerOps*>(&m_vtable)
        );
    }

private:
    ::TFGrapplerOptimizerOps m_vtable;
    ::TFGrapplerOptimizer m_handle;

    const ::TFGrapplerItemOps* m_TFGrapplerItemOps_ops{nullptr};

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

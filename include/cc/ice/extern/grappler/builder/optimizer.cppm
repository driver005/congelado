// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/optimizer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_grappler_builder:optimizer;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GrapplerOptimizer
{
public:
    explicit GrapplerOptimizer(
        const ::TFGrapplerItemOps* GrapplerItem_ops,
        const ::TF_BufferOps* Buffer_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_GrapplerItem_ops = GrapplerItem_ops;
        m_Buffer_ops = Buffer_ops;
        m_Status_ops = Status_ops;
    }

    GrapplerOptimizer(const GrapplerOptimizer&) = delete;
    GrapplerOptimizer& operator=(const GrapplerOptimizer&) = delete;

    static GrapplerOptimizer& from_handle(void* ctx) noexcept
    {
        return *static_cast<GrapplerOptimizer*>(ctx);
    }

    template<typename HandleT>
    static GrapplerOptimizer& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GrapplerOptimizer*>(handle->plugin_data);
    }

    virtual ~GrapplerOptimizer() = default;
    virtual void destroy() noexcept = 0;
    virtual void optimize(
        const ice::sonic::Buffer& graph_buf,
        const ice::sonic::GrapplerItem& item,
        const ice::sonic::Buffer& out_optimized_graph_buf,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerOptimizer*)) noexcept
    {
        m_vtable = ::TFGrapplerOptimizerOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerOptimizerOps, optimize),

            .create = create,
            .destroy =
                [](::TFGrapplerOptimizer* handle) noexcept
            {
                auto& self = GrapplerOptimizer::from_handle(handle);
                self.destroy();
            },
            .optimize =
                [](::TFGrapplerOptimizer* optimizer,
                   const ::TF_Buffer* graph_buf,
                   const ::TFGrapplerItem* item,
                   ::TF_Buffer* out_optimized_graph_buf,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerOptimizer::from_handle(optimizer);
                self.optimize(
                    self.wrap(std::type_identity<ice::sonic::Buffer>{}, graph_buf),
                    self.wrap(std::type_identity<ice::sonic::GrapplerItem>{}, item),
                    self.wrap(std::type_identity<ice::sonic::Buffer>{}, out_optimized_graph_buf),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::GrapplerItem wrap(
        std::type_identity<ice::sonic::GrapplerItem>,
        const ::TFGrapplerItem* handle
    ) const noexcept
    {
        return ice::sonic::GrapplerItem{m_GrapplerItem_ops, const_cast<::TFGrapplerItem*>(handle)};
    }

    ice::sonic::Buffer
    wrap(std::type_identity<ice::sonic::Buffer>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::Buffer{m_Buffer_ops, const_cast<::TF_Buffer*>(handle)};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGrapplerOptimizerOps*>(&m_vtable));
    }

private:
    ::TFGrapplerOptimizerOps m_vtable;
    ::TFGrapplerOptimizer m_handle;

    const ::TFGrapplerItemOps* m_GrapplerItem_ops{nullptr};

    const ::TF_BufferOps* m_Buffer_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder

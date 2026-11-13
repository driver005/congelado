module;

#include "include/c/extern/gradients/gradient.h"

export module aten_xpu_extern_gradients:catalog;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_gradients_builder;
import aten_xpu_intern;
import :gradient;
import :math_add;
import :activation_bias_add;
import :math_div_no_nan;
import :math_exp;
import :math_identity_n;
import :math_log1p;
import :matmul_matmul;
import :math_multiply;
import :math_negate;
import :math_not_differentiable;
import :activation_relu;
import :activation_sparse_softmax_cross_entropy;
import :math_sqrt;
import :math_subtract;
import :matmul_addmm;
import :attention_attention;
import :convolution_convolution;
import :copy_copy;
import :matmul_linear;
import :transpose_transpose;

export namespace aten_xpu {

// The gradients the plugin provides: TensorFlow's set plus one for each differentiable plugin op.
// Each is registered in the global registry with type "gradient" and the op it differentiates as
// provider; the host creates one per use from the ops table found there.
class SyclGradientCatalog
{
public:
    explicit SyclGradientCatalog(const SyclOpsTable& ops) :
        m_ops{ops},
        m_type{ops.getStringOps()},
        m_provider{ops.getStringOps()}
    {
        m_type.create();
        m_provider.create();
        m_type.copy("gradient");
    }

    ~SyclGradientCatalog()
    {
        m_provider.destroy();
        m_type.destroy();
    }

    SyclGradientCatalog(const SyclGradientCatalog&) = delete;
    SyclGradientCatalog& operator=(const SyclGradientCatalog&) = delete;
    SyclGradientCatalog(SyclGradientCatalog&&) = delete;
    SyclGradientCatalog& operator=(SyclGradientCatalog&&) = delete;

    void registerAll(const ice::sonic::Registration& registry)
    {
        add<SyclAddGradient>(registry, "Add");
        add<SyclAddGradient>(registry, "AddV2");
        add<SyclSubtractGradient>(registry, "Sub");
        add<SyclNegateGradient>(registry, "Neg");
        add<SyclMultiplyGradient>(registry, "Mul");
        add<SyclExpGradient>(registry, "Exp");
        add<SyclSqrtGradient>(registry, "Sqrt");
        add<SyclLog1pGradient>(registry, "Log1p");
        add<SyclDivNoNanGradient>(registry, "DivNoNan");
        add<SyclReluGradient>(registry, "Relu");
        add<SyclMatMulGradient>(registry, "MatMul");
        add<SyclBiasAddGradient>(registry, "BiasAdd");
        add<SyclIdentityNGradient>(registry, "IdentityN");
        add<SyclSparseSoftmaxCrossEntropyGradient>(registry, "SparseSoftmaxCrossEntropyWithLogits");
        add<SyclAddmmGradient>(registry, "Addmm");
        add<SyclLinearGradient>(registry, "Linear");
        add<SyclTransposeGradient>(registry, "Transpose");
        add<SyclCopyGradient>(registry, "_CopyFromXpuToHost");
        add<SyclCopyGradient>(registry, "_CopyFromHostToXpu");
        add<SyclConvolutionGradient>(registry, "Conv2D");
        add<SyclConvolutionGradient>(registry, "Conv3D");
        add<SyclAttentionGradient>(registry, "ScaledDotProductAttention");
        for (const auto op_name:
             {"ConvolutionBackward",
              "ScaledDotProductAttentionGrad",
              "_ScopedAllocator",
              "_ScopedAllocatorConcat",
              "_ScopedAllocatorSplit"}) {
            add<SyclNotDifferentiableGradient>(registry, op_name);
        }
    }

    // The gradient the registry hands out for an empty provider: its first registration.
    const ::TFGradientOps* findDefault(const ice::sonic::Registration& registry) const
    {
        void* found = nullptr;
        m_provider.copy("");
        registry.get(m_type, m_provider, &found);

        return static_cast<const ::TFGradientOps*>(found);
    }

    std::size_t getCount() const noexcept
    {
        return m_prototypes.size();
    }

private:
    template<typename Gradient>
    void add(const ice::sonic::Registration& registry, std::string_view op_name)
    {
        auto prototype = std::make_unique<Gradient>(m_ops);
        prototype->get_generic_vtable(
            [](::TFGradient* handle)
            {
                auto* object = new Gradient{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_provider.copy(op_name);
        prototype->register_ops(registry, m_type, m_provider);
        m_prototypes.push_back(std::move(prototype));
    }

    const SyclOpsTable& m_ops;
    ice::sonic::String m_type;
    ice::sonic::String m_provider;
    std::vector<std::unique_ptr<ice::builder::Gradient>> m_prototypes;
};

} // namespace aten_xpu

module;

export module aten_xpu_extern_kernel:ops_catalog;

import std;
import :ops_shape_inference;
import :ops_registrar;

export namespace aten_xpu {

// The ops the plugin's kernels implement; the host owns the registry, so the plugin defines each op
// under the name its kernel registers with.
class SyclOpCatalog
{
public:
    explicit SyclOpCatalog(SyclOpRegistrar& registrar) noexcept :
        m_registrar{registrar}
    {
    }

    SyclOpCatalog(const SyclOpCatalog&) = delete;
    SyclOpCatalog& operator=(const SyclOpCatalog&) = delete;
    SyclOpCatalog(SyclOpCatalog&&) = delete;
    SyclOpCatalog& operator=(SyclOpCatalog&&) = delete;

    bool registerAll()
    {
        bool registered = true;
        registered &= m_registrar.add<SyclShapeRule::k_addend_matmul>(
            "Addmm",
            {k_float_types, "alpha: float = 1.0", "beta: float = 1.0", "activation: int = 0"},
            {"addend: T", "first: T", "second: T"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_addend_matrix_vector>(
            "Addmv",
            {k_float_types, "alpha: float = 1.0", "beta: float = 1.0", "activation: int = 0"},
            {"addend: T", "matrix: T", "vector: T"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_addend_batch_matmul>(
            "Baddbmm",
            {k_float_types, "alpha: float = 1.0", "beta: float = 1.0", "activation: int = 0"},
            {"addend: T", "first: T", "second: T"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_linear>(
            "Linear",
            {k_float_types,
             "unary: string = 'none'",
             "algorithm: string = 'none'",
             "binary: string = 'none'",
             "scalars: list(float) = []",
             "activation: int = 0",
             "Textras: list(type) >= 0 = []"},
            {"input: T", "weight: T", "extras: Textras"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_convolution_gradient>(
            "ConvolutionBackward",
            {k_float_types,
             "stride: list(int) = []",
             "padding: list(int) = []",
             "output_padding: list(int) = []",
             "dilation: list(int) = []",
             "groups: int = 1",
             "input_gradient: bool = true",
             "weight_gradient: bool = true",
             "bias_gradient: bool = true"},
            {"gradient_output: T", "source: T", "weight: T"},
            {"source_derivative: T", "weight_derivative: T", "bias_derivative: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_transposed_convolution>(
            "Conv2DBackpropInputTransposed",
            {k_float_types,
             "stride: list(int) = []",
             "padding: list(int) = []",
             "output_padding: list(int) = []",
             "dilation: list(int) = []",
             "groups: int = 1",
             "unary: string = 'none'",
             "algorithm: string = 'none'",
             "binary: string = 'none'",
             "scalars: list(float) = []",
             "activation: int = 0",
             "Textras: list(type) >= 0 = []"},
            {"source: T", "weight: T", "extras: Textras"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_weight_only_matmul>(
            "WeightOnlyQuantizedMatMul",
            {k_float_types,
             "Tweight: type",
             "group_size: int = 128",
             "symmetric: bool = false",
             "Tscales: list(type) >= 1"},
            {"activation: T", "packed_weight: Tweight", "scales: Tscales"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_attention>(
            "ScaledDotProductAttention",
            {k_float_types,
             "scale: float = 0.0",
             "dropout: float = 0.0",
             "is_causal: bool = false",
             "return_probabilities: bool = false",
             "Textras: list(type) >= 0 = []",
             "Tprobabilities: list(type) >= 0 = []"},
            {"query: T", "key: T", "value: T", "extras: Textras"},
            {"output: T", "probabilities: Tprobabilities"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_attention_gradient>(
            "ScaledDotProductAttentionGrad",
            {k_float_types,
             "scale: float = 0.0",
             "is_causal: bool = false",
             "Textras: list(type) >= 0 = []"},
            {"gradient_output: T", "query: T", "key: T", "value: T", "extras: Textras"},
            {"query_derivative: T", "key_derivative: T", "value_derivative: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_recurrent>(
            "LSTM",
            {k_float_types,
             "num_layers: int = 1",
             "bidirectional: bool = false",
             "Tweights: list(type) >= 0"},
            {"input: T", "initial_hidden: T", "initial_cell: T", "weights: Tweights"},
            {"output: T", "final_hidden: T", "final_cell: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_matmul>(
            "ScaledMatMul",
            {"T: {float8_e4m3fn, float8_e5m2}",
             "output_dtype: type = DT_BFLOAT16",
             "use_fast_accum: bool = false",
             "Tscales: list(type) >= 2"},
            {"left: T", "right: T", "scales: Tscales"},
            {"output: output_dtype"}
        );
        for (const auto name: {"QuantizedLinear", "QuantizedMatMul"}) {
            registered &= m_registrar.add<SyclShapeRule::k_quantized_matmul>(
                name,
                {"T: {qint8, quint8}",
                 "Tweight: type",
                 "output_dtype: type = DT_QINT8",
                 "weight_transposed: bool = false",
                 "unary: string = 'none'",
                 "algorithm: string = 'none'",
                 "binary: string = 'none'",
                 "scalars: list(float) = []",
                 "other_scale: float = 1.0",
                 "other_zero_point: int = 0",
                 "Tscales: list(type) >= 3"},
                {"source: T", "weight: Tweight", "scales: Tscales"},
                {"output: output_dtype"}
            );
        }
        registered &= m_registrar.add<SyclShapeRule::k_batch_matmul>(
            "BatchMatMul",
            {k_float_types, "alpha: float = 1.0", "beta: float = 0.0", "activation: int = 0"},
            {"first: T", "second: T"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_convolution>(
            "Conv2D",
            {k_float_types,
             "stride: list(int) = []",
             "padding: list(int) = []",
             "output_padding: list(int) = []",
             "dilation: list(int) = []",
             "groups: int = 1",
             "unary: string = 'none'",
             "algorithm: string = 'none'",
             "binary: string = 'none'",
             "scalars: list(float) = []",
             "activation: int = 0",
             "Textras: list(type) >= 0 = []"},
            {"source: T", "weight: T", "extras: Textras"},
            {"output: T"}
        );
        registered &= m_registrar.add<SyclShapeRule::k_convolution>(
            "QuantizedConv2D",
            {"T: {qint8, quint8}",
             "Tweight: type",
             "stride: list(int) = []",
             "padding: list(int) = []",
             "output_padding: list(int) = []",
             "dilation: list(int) = []",
             "groups: int = 1",
             "output_dtype: type = DT_QINT8",
             "unary: string = 'none'",
             "algorithm: string = 'none'",
             "binary: string = 'none'",
             "scalars: list(float) = []",
             "accumulator_scale: float = 1.0",
             "accumulator_zero_point: int = 0",
             "Tscales: list(type) >= 3"},
            {"source: T", "weight: Tweight", "scales: Tscales"},
            {"output: output_dtype"}
        );
        return registered;
    }

private:
    static constexpr std::string_view k_float_types = "T: {float, half, bfloat16}";

    SyclOpRegistrar& m_registrar;
};

} // namespace aten_xpu

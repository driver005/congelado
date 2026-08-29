module;

export module cc_abi_builder_generator:definition;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :parameter;
import :attribute;

export namespace ice::builder::generator {

// Abstract base class for generator definition view.
// Inputs, outputs and attrs are exposed as ice::TensorHandle — the implementation
// allocates via the injected m_tensor_runtime and returns the handle directly.
class Definition
{
public:
    // Tensor runtime injected at construction so implementations of
    // get_inputs/outputs/attrs can allocate tensors through it.
    explicit Definition(ice::sonic::Tensor& tensor_runtime)
        : m_tensor_runtime{tensor_runtime}
    {}

    virtual ~Definition() = default;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_summary() const = 0;
    virtual ice::String get_description() const = 0;

    // Each returns a TensorHandle the implementation allocates via m_tensor_runtime.
    // The out parameter is an optional pre-allocated handle the caller may supply;
    // implementations may ignore it and return a freshly allocated one.
    virtual std::expected<ice::TensorHandle, ice::Status>
    get_inputs(ice::TensorHandle out) const = 0;

    virtual std::expected<ice::TensorHandle, ice::Status>
    get_outputs(ice::TensorHandle out) const = 0;

    virtual std::expected<ice::TensorHandle, ice::Status>
    get_attrs(ice::TensorHandle out) const = 0;

protected:
    ice::sonic::Tensor& m_tensor_runtime;
};

} // namespace ice::builder::generator

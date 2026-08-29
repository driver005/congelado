module;

export module cc_abi_builder_generator:definition;

import std;
import cc_abi_primitives;
import cc_abi_builder_intern;
import cc_abi_sonic_intern;
import :parameter;
import :attribute;

export namespace ice::builder {

// Abstract base class for generator definition view.
// Inputs, outputs and attrs are exposed as ice::TensorHandle — the implementation
// allocates via the injected m_tensor_runtime and returns the handle directly.
class Definition
{
public:
    // Recover the Definition instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Definition* create(void* ctx) noexcept
    {
        return static_cast<Definition*>(ctx);
    }

    // Const overload for read-only vtable callbacks that receive a const void*.
    static const Definition* create(const void* ctx) noexcept
    {
        return static_cast<const Definition*>(ctx);
    }

    // Tensor runtime injected at construction so implementations of
    // get_inputs/outputs/attrs can allocate tensors through it. Optional (default
    // ctor): tensor-returning methods then fail with a clear Status.
    Definition() = default;

    explicit Definition(ice::sonic::Tensor& tensor_runtime) :
        m_tensor_runtime{&tensor_runtime}
    {
    }

    // Assigns the tensor runtime after default construction — lets owning objects
    // (e.g. a Builder handing out definition copies) equip their definitions for
    // tensor allocation.
    void set_tensor_runtime(ice::sonic::Tensor& tensor_runtime) noexcept
    {
        m_tensor_runtime = &tensor_runtime;
    }

    virtual ~Definition() = default;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_summary() const = 0;
    virtual ice::String get_description() const = 0;

    // Each returns a TensorHandle the implementation allocates via m_tensor_runtime.
    // The out parameter is an optional pre-allocated handle the caller may supply;
    // implementations may ignore it and return a freshly allocated one.
    virtual [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_inputs(ice::TensorHandle out) const = 0;

    virtual [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_outputs(ice::TensorHandle out) const = 0;

    virtual [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    get_attrs(ice::TensorHandle out) const = 0;

protected:
    // Allocates a 1-D Uint8 tensor of `count` opaque-handle slots via m_tensor_runtime
    // (the list/array-carrier contract of TF_Tensor_Handle). The caller fills the
    // data buffer (count*sizeof(void*) bytes) with its handles. Fails with a clear
    // Status when no tensor runtime is available or allocation fails.
    [[nodiscard]] std::expected<ice::TensorHandle, ice::Status>
    make_handle_tensor(int64_t count) const
    {
        if (!m_tensor_runtime) {
            return std::unexpected{ice::Status{"no tensor runtime available"}};
        }
        size_t bytes = static_cast<size_t>(count) * sizeof(void*);
        auto res = m_tensor_runtime->allocate_tensor(ice::DataTypeEnum::Uint8, &count, 1, bytes);
        if (!res) {
            return std::unexpected{res.error()};
        }
        auto* raw = res.value();
        return ice::TensorHandle{raw};
    }

    ice::sonic::Tensor* m_tensor_runtime = nullptr;
};

} // namespace ice::builder

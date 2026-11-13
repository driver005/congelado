module;

#include "include/c/extern/gradients/context.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_extern_gradients:emitter;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_gradients_sonic;
import aten_xpu_intern;
import :value;

export namespace aten_xpu {

using SyclGradientInputs = std::initializer_list<std::reference_wrapper<const SyclGradientValue>>;

// One gradient computation seen from the plugin: reads the forward op, builds the ops that compute
// each input gradient on the host and hands the results back. It replaces TensorFlow's
// ForwardOperation, AbstractContext and ops:: wrappers.
class SyclGradientEmitter
{
public:
    SyclGradientEmitter(
        const ice::sonic::GradientContext& context,
        const ice::sonic::Status& status
    ) :
        m_ops{SyclOpsTable::getInstance()},
        m_helper{m_ops},
        m_context{context},
        m_status{status},
        m_first{m_ops.getStringOps()},
        m_second{m_ops.getStringOps()},
        m_element{ice::sonic::DataTypeEnum::k_float}
    {
        m_first.create();
        m_second.create();
        m_context.get_name(m_first);
        m_forward_name.assign(String::view_of(m_first));
        if (hasAttribute("T")) {
            readType("T", m_element);
        }
    }

    ~SyclGradientEmitter()
    {
        m_second.destroy();
        m_first.destroy();
    }

    SyclGradientEmitter(const SyclGradientEmitter&) = delete;
    SyclGradientEmitter& operator=(const SyclGradientEmitter&) = delete;
    SyclGradientEmitter(SyclGradientEmitter&&) = delete;
    SyclGradientEmitter& operator=(SyclGradientEmitter&&) = delete;

    bool ok() const noexcept
    {
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);

        return code.get_value() == TF_OK;
    }

    void fail(ice::sonic::Code code, std::string_view message) const noexcept
    {
        m_helper.fail(m_status, code, message);
    }

    std::string_view getForwardName() const noexcept
    {
        return m_forward_name;
    }

    ice::sonic::DataTypeEnum getElementType() const noexcept
    {
        return m_element;
    }

    int getInputCount() const noexcept
    {
        int count = 0;
        m_context.num_inputs(count);

        return count;
    }

    int getUpstreamCount() const noexcept
    {
        int count = 0;
        m_context.num_grad_outputs(count);

        return count;
    }

    int getResultCount() const noexcept
    {
        int count = 0;
        m_context.num_grad_inputs(count);

        return count;
    }

    SyclGradientValue input(int index)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.get_input(index, &handle, m_status);

        return adopt(handle);
    }

    SyclGradientValue output(int index)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.get_output(index, &handle, m_status);

        return adopt(handle);
    }

    // The gradient flowing into the forward op's output at index.
    SyclGradientValue upstream(int index)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.get_grad_output(index, &handle, m_status);

        return adopt(handle);
    }

    SyclGradientValue constant(double value, ice::sonic::DataTypeEnum type)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.constant_scalar(value, type, &handle, m_status);

        return adopt(handle);
    }

    SyclGradientValue constant(double value)
    {
        return constant(value, m_element);
    }

    bool hasAttribute(std::string_view name)
    {
        bool found = false;
        assign(m_first, name);
        m_context.has_attr(m_first, found, m_status);

        return ok() && found;
    }

    bool readBool(std::string_view name, bool fallback)
    {
        if (!hasAttribute(name)) {
            return fallback;
        }
        bool value = fallback;
        assign(m_first, name);
        m_context.get_attr_bool(m_first, value, m_status);

        return value;
    }

    std::int64_t readInt(std::string_view name, std::int64_t fallback)
    {
        if (!hasAttribute(name)) {
            return fallback;
        }
        std::int64_t value = fallback;
        assign(m_first, name);
        m_context.get_attr_int64(m_first, value, m_status);

        return value;
    }

    float readFloat(std::string_view name, float fallback)
    {
        if (!hasAttribute(name)) {
            return fallback;
        }
        float value = fallback;
        assign(m_first, name);
        m_context.get_attr_float(m_first, value, m_status);

        return value;
    }

    bool readType(std::string_view name, ice::sonic::DataTypeEnum& value)
    {
        assign(m_first, name);
        m_context.get_attr_type(m_first, value, m_status);

        return ok();
    }

    std::string_view readString(std::string_view name, std::string_view fallback)
    {
        if (!hasAttribute(name)) {
            return fallback;
        }
        assign(m_first, name);
        m_context.get_attr_string(m_first, m_second, m_status);
        m_scratch_text.assign(String::view_of(m_second));

        return m_scratch_text;
    }

    // The list attribute; the view stays valid until the next list is read.
    const std::vector<std::int64_t>& readIntegers(std::string_view name)
    {
        m_scratch_integers.clear();
        if (!hasAttribute(name)) {
            return m_scratch_integers;
        }
        std::int32_t list_size = 0;
        std::int32_t total_size = 0;
        assign(m_first, name);
        m_context.get_attr_size(m_first, list_size, total_size, m_status);
        if (!ok() || list_size <= 0) {
            return m_scratch_integers;
        }
        m_scratch_integers.resize(static_cast<std::size_t>(list_size));
        m_context.get_attr_int64_list(m_first, m_scratch_integers, m_status);

        return m_scratch_integers;
    }

    bool begin(std::string_view op)
    {
        if (!ok()) {
            return false;
        }
        m_scratch_text.assign(std::format("gradient/{}/{}_{}", m_forward_name, op, m_counter++));
        assign(m_first, op);
        assign(m_second, m_scratch_text);
        m_context.begin_op(m_first, m_second, m_status);

        return ok();
    }

    void add(const SyclGradientValue& value)
    {
        if (ok()) {
            m_context.add_input(value.wrap(), m_status);
        }
    }

    void setType(std::string_view name, ice::sonic::DataTypeEnum value)
    {
        assign(m_first, name);
        m_context.set_attr_type(m_first, value, m_status);
    }

    void setBool(std::string_view name, bool value)
    {
        assign(m_first, name);
        m_context.set_attr_bool(m_first, value, m_status);
    }

    void setInt(std::string_view name, std::int64_t value)
    {
        assign(m_first, name);
        m_context.set_attr_int64(m_first, value, m_status);
    }

    void setFloat(std::string_view name, float value)
    {
        assign(m_first, name);
        m_context.set_attr_float(m_first, value, m_status);
    }

    void setString(std::string_view name, std::string_view value)
    {
        assign(m_first, name);
        assign(m_second, value);
        m_context.set_attr_string(m_first, m_second, m_status);
    }

    void setIntegers(std::string_view name, const std::vector<std::int64_t>& values)
    {
        assign(m_first, name);
        m_context.set_attr_int64_list(m_first, values, m_status);
    }

    // Closes the op being built; the number of outputs it produced, zero on failure.
    int finish()
    {
        int count = 0;
        if (ok()) {
            m_context.finish_op(count, m_status);
        }

        return ok() ? count : 0;
    }

    SyclGradientValue result(int index)
    {
        ::TF_Tensor* handle = nullptr;
        m_context.get_op_output(index, &handle, m_status);

        return adopt(handle);
    }

    // An op with one output and the forward element type in attribute T.
    SyclGradientValue call(std::string_view op, SyclGradientInputs inputs)
    {
        if (!begin(op)) {
            return SyclGradientValue{};
        }
        for (const auto& value: inputs) {
            add(value.get());
        }
        setType("T", m_element);

        return finish() > 0 ? result(0) : SyclGradientValue{};
    }

    // MatMul with the transpose flags the host op carries.
    SyclGradientValue matmul(
        const SyclGradientValue& first,
        const SyclGradientValue& second,
        bool transpose_first,
        bool transpose_second
    )
    {
        if (!begin("MatMul")) {
            return SyclGradientValue{};
        }
        add(first);
        add(second);
        setType("T", m_element);
        setBool("transpose_a", transpose_first);
        setBool("transpose_b", transpose_second);

        return finish() > 0 ? result(0) : SyclGradientValue{};
    }

    // Sums gradient down to the shape of like, the way TensorFlow undoes a broadcast.
    SyclGradientValue reduceTo(const SyclGradientValue& gradient, const SyclGradientValue& like)
    {
        const auto gradient_shape = call("Shape", {gradient});
        const auto like_shape = call("Shape", {like});
        if (!gradient_shape || !like_shape || !begin("BroadcastGradientArgs")) {
            return SyclGradientValue{};
        }
        add(like_shape);
        add(gradient_shape);
        setType("T", ice::sonic::DataTypeEnum::k_int32);
        if (finish() < 1) {
            return SyclGradientValue{};
        }
        const auto axes = result(0);
        if (!begin("Sum")) {
            return SyclGradientValue{};
        }
        add(gradient);
        add(axes);
        setType("T", m_element);
        setType("Tidx", ice::sonic::DataTypeEnum::k_int32);
        setBool("keep_dims", false);
        const auto summed = finish() > 0 ? result(0) : SyclGradientValue{};
        if (!summed || !begin("Reshape")) {
            return SyclGradientValue{};
        }
        add(summed);
        add(like_shape);
        setType("T", m_element);
        setType("Tshape", ice::sonic::DataTypeEnum::k_int32);

        return finish() > 0 ? result(0) : SyclGradientValue{};
    }

    // Hands the gradient with respect to forward input index back to the host.
    bool setResult(int index, const SyclGradientValue& value)
    {
        if (!value || !ok()) {
            return false;
        }
        m_context.set_grad_input(index, value.wrap(), m_status);

        return ok();
    }

private:
    SyclGradientValue adopt(::TF_Tensor* handle) const
    {
        if (!ok() || handle == nullptr) {
            return SyclGradientValue{};
        }

        return SyclGradientValue{handle, m_ops.getTensorOps()};
    }

    static void assign(const ice::sonic::String& target, std::string_view text)
    {
        target.copy(text);
    }

    const SyclOpsTable& m_ops;
    SyclStatus m_helper;
    const ice::sonic::GradientContext& m_context;
    const ice::sonic::Status& m_status;
    ice::sonic::String m_first;
    ice::sonic::String m_second;
    ice::sonic::DataTypeEnum m_element;
    std::string m_forward_name;
    std::string m_scratch_text;
    std::vector<std::int64_t> m_scratch_integers;
    std::size_t m_counter{0};
};

} // namespace aten_xpu

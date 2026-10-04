module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:scaled_scaling_recipe;

import std;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclScalingRecipe
{
public:
    enum class Kind : std::uint8_t
    {
        tensor_wise,
        row_wise,
        block_wise_1x128,
        block_wise_128x128,
        block_wise_1x32,
        unsupported
    };

    static Kind detect(const SyclTensor& scale, int64_t rows, int64_t inner, bool is_left_operand) noexcept
    {

        const auto count = scale.element_count();
        const auto outer = is_left_operand ? rows : inner;
        const auto reduction = is_left_operand ? inner : rows;
        if (count == 1) {
            return Kind::tensor_wise;
        }
        if (count == outer) {
            return Kind::row_wise;
        }
        if (count == outer * ceil_div(reduction, 128)) {
            return Kind::block_wise_1x128;
        }
        if (count == ceil_div(outer, 128) * ceil_div(reduction, 128)) {
            return Kind::block_wise_128x128;
        }
        if (count == outer * ceil_div(reduction, 32)) {
            return Kind::block_wise_1x32;
        }
        return Kind::unsupported;

    }

    static void apply(dnnl::primitive_attr& attributes, int argument, Kind kind, bool is_left_operand)
    {

        const int outer_mask = is_left_operand ? 1 << 0 : 1 << 1;
        switch (kind) {
            case Kind::tensor_wise:
                attributes.set_scales_mask(argument, 0);
                return;
            case Kind::row_wise:
                attributes.set_scales_mask(argument, outer_mask);
                return;
            case Kind::block_wise_1x128:
                attributes.set_scales(argument, 0b11, group_dims(1, 128, is_left_operand), dnnl::memory::data_type::f32);
                return;
            case Kind::block_wise_128x128:
                attributes.set_scales(argument, 0b11, group_dims(128, 128, is_left_operand), dnnl::memory::data_type::f32);
                return;
            case Kind::block_wise_1x32:
                attributes.set_scales(argument, 0b11, group_dims(1, 32, is_left_operand), dnnl::memory::data_type::e8m0);
                return;
            case Kind::unsupported:
                return;
        }

    }

    static bool compatible(Kind left, Kind right) noexcept
    {

        if (left == Kind::unsupported || right == Kind::unsupported) {
            return false;
        }
        if (left == Kind::tensor_wise || left == Kind::row_wise) {
            return right == left;
        }
        return right != Kind::tensor_wise && right != Kind::row_wise;

    }

private:
    static int64_t ceil_div(int64_t value, int64_t divisor) noexcept { return (value + divisor - 1) / divisor; }

    static dnnl::memory::dims group_dims(int64_t outer, int64_t reduction, bool is_left_operand)
    {

        return is_left_operand ? dnnl::memory::dims{outer, reduction} : dnnl::memory::dims{reduction, outer};

    }
};

} // namespace aten_xpu

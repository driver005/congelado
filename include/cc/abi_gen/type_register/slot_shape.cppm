export module cc_abi_gen_type_register:slot_shape;

import std;
import cc_abi_gen_parser;
import :type_rule;
import :registry;

export namespace cc_abi_gen::type_register {

// Which hand-written idiom a slot's generated method/vtable-field should follow, classified by
// shape (parameter count/constness/failability) rather than by slot name — replaces the old
// pure name-based is_get_name() special case with a generalization that also covers
// get_content_type/get_format_name/get_device_type (StringAccessor), encode/decode (CodecPair),
// and collect_data_xspace (TypedReturn).
enum class SlotShape
{
    Destroy,
    StringAccessor,
    CodecPair,
    TypedReturn,
    Generic
};

namespace detail {

    inline bool is_const_pointer_type(std::string_view raw_type) noexcept
    {
        return raw_type.starts_with("const ");
    }

    // "TF_Tensor_Handle *" -> "TF_Tensor_Handle"; "const TF_TString *" -> "TF_TString"
    inline std::string strip_pointer_type(std::string_view raw_type) noexcept
    {
        std::string result{raw_type};

        while (!result.empty() && result.back() == ' ') {
            result.pop_back();
        }
        if (!result.empty() && result.back() == '*') {
            result.pop_back();
            while (!result.empty() && result.back() == ' ') {
                result.pop_back();
            }
        }
        if (result.starts_with("const ")) {
            result.erase(0, 6);
        }

        return result;
    }

} // namespace detail

inline SlotShape classify_slot(const parser::slot::Slot& slot, const TypeRegister& registry) noexcept
{
    if (slot.is_destroy()) {
        return SlotShape::Destroy;
    }

    auto params = slot.extract_parameters();

    if (params.size() == 1 && !slot.is_failable()) {
        const auto& param = params[0];

        if (!param.get_pointee_name().empty() && !detail::is_const_pointer_type(param.get_type()))
        {
            auto rule = registry.find_type_rule(param.get_pointee_name());
            if (rule.has_value() && rule->get().get_class_name() == "String") {
                return SlotShape::StringAccessor;
            }
        }
    }

    if (params.size() == 2 && slot.is_failable()) {
        const auto& in_param = params[0];
        const auto& out_param = params[1];

        if (detail::is_const_pointer_type(in_param.get_type())
            && !detail::is_const_pointer_type(out_param.get_type()))
        {
            auto in_rule = registry.find_type_rule(in_param.get_pointee_name());
            auto out_rule = registry.find_type_rule(out_param.get_pointee_name());

            if (in_rule.has_value() && out_rule.has_value()
                && in_rule->get().get_class_name() == out_rule->get().get_class_name())
            {
                return SlotShape::CodecPair;
            }
        }
    }

    if (params.empty() && slot.is_failable() && slot.get_return_type() != "void") {
        std::string pointee = detail::strip_pointer_type(slot.get_return_type());

        if (registry.find_type_rule(pointee).has_value()) {
            return SlotShape::TypedReturn;
        }
    }

    return SlotShape::Generic;
}

} // namespace cc_abi_gen::type_register

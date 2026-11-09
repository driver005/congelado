export module cc_abi_gen_generator:utils;

import std;
import cc_abi_gen_parser;

export namespace cc_abi_gen::generator::utils {

// "TFGrapplerConfigsOps" -> "TFGrapplerConfigs" when a slot takes that handle
constexpr std::string c_handle_name(const parser::vtable::Model& model)
{
    std::string_view struct_name = model.get_struct_name();
    if (!struct_name.ends_with("Ops")) {
        return {};
    }

    std::string handle_name{struct_name.substr(0, struct_name.size() - 3)};
    for (const parser::slot::Slot& slot: model.get_slots()) {
        auto parameters = slot.get_parameters();
        if (!parameters.empty() && parameters.front().get_pointee_name() == handle_name) {
            return handle_name;
        }
    }

    return {};
}

} // namespace cc_abi_gen::generator::utils

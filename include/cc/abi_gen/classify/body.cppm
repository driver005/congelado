export module cc_abi_gen_classify:body;

import std;
import :parms;

export namespace cc_abi_gen::classify {

class BodyBuilder
{
public:
    static constexpr std::string_view INDENT = "                ";

    BodyBuilder() = default;
    ~BodyBuilder() = default;

    BodyBuilder(const BodyBuilder&) = delete;
    BodyBuilder& operator=(const BodyBuilder&) = delete;
    BodyBuilder(BodyBuilder&&) noexcept = default;
    BodyBuilder& operator=(BodyBuilder&&) noexcept = default;

    // `m_vtable = ::Struct{ .struct_size = MACRO, …`
    BodyBuilder& begin_vtable(std::string struct_name, std::string size_macro)
    {
        if (m_vtable_open || m_slot_open) {
            return *this;
        }

        m_vtable_open = true;
        m_text += std::format("m_vtable = ::{}{{\n", struct_name);
        m_text += std::format("    .struct_size = {},\n", size_macro);
        m_text += '\n';
        return *this;
    }

    // `.name = [](params) noexcept {` — params/body collected until end_slot.
    BodyBuilder& begin_slot(std::string slot_name)
    {
        if (!m_vtable_open || m_slot_open) {
            return *this;
        }

        m_slot_open = true;
        m_slot_name = std::move(slot_name);
        m_slot_params.clear();
        m_body.clear();
        return *this;
    }

    // `},` — closes the open slot, appends it to the aggregate.
    BodyBuilder& end_slot()
    {
        if (!m_slot_open) {
            return *this;
        }

        std::string params = m_slot_params |
                             std::views::transform(
                                 [](const Param& param)
                                 {
                                     return param.to_string();
                                 }
                             ) |
                             std::views::join_with(std::string_view(", ")) |
                             std::ranges::to<std::string>();

        if (m_vtable_open) {
            m_text += "    ";
        }
        m_text += std::format(".{} = []({}) noexcept\n{{\n", m_slot_name, params);
        m_text += m_body;
        m_text += "},\n";

        m_slot_open = false;
        m_slot_name.clear();
        m_slot_params.clear();
        m_body.clear();
        return *this;
    }

    // `};` — closes the aggregate.
    BodyBuilder& end_vtable()
    {
        if (!m_vtable_open || m_slot_open) {
            return *this;
        }
        m_vtable_open = false;
        m_text += "\n};\n";
        return *this;
    }

    // A plain designated entry, e.g. `.create = create`.
    BodyBuilder& add_entry(const std::string& expression)
    {
        if (!m_vtable_open || m_slot_open) {
            return *this;
        }

        if (m_vtable_open) {
            m_text += "    ";
        }
        m_text += expression;
        m_text += ",\n";
        return *this;
    }

    // insert with own indentation
    BodyBuilder& add_raw(const std::string& block)
    {
        m_body += block;
        return *this;
    }

    // `auto& self = Class::from_handle(param);`
    BodyBuilder& add_self_handle(std::string_view class_name, std::string_view param_name)
    {
        m_body +=
            std::format("{}auto& self = {}::from_handle({});\n", INDENT, class_name, param_name);
        return *this;
    }

    // `m_ops->method(get_handle(), args);` — `, ` only when has_arguments.
    BodyBuilder&
    add_ops_call(std::string_view method, std::string_view arguments, bool has_arguments)
    {
        m_body += std::format("{}m_ops->{}(get_handle()", INDENT, method);
        if (has_arguments) {
            m_body += ", ";
        }
        m_body += arguments;
        m_body += ");\n";
        return *this;
    }

    // `self.method(args);`
    BodyBuilder& add_self_call(std::string_view method, std::string_view arguments)
    {
        m_body += std::format("{}self.{}(", INDENT, method);
        m_body += arguments;
        m_body += ");\n";
        return *this;
    }

    // Parameter of the open slot.
    BodyBuilder& add_param(Param param)
    {
        if (!m_slot_open) {
            return *this;
        }
        m_slot_params.push_back(std::move(param));
        return *this;
    }

    [[nodiscard]] std::expected<std::string, std::string> take()
    {
        // Refuse half-built output: aggregate or slot still open.
        if (m_vtable_open) {
            return std::unexpected{"Please close the vtable first befor extracting the code"};
        }

        if (m_slot_open) {
            return std::unexpected{"Please close the last slot first befor extracting the code"};
        }

        std::string result = std::move(m_text);

        m_text.clear();

        return result;
    }

private:
    // finished
    std::string m_text;
    // current open body
    std::string m_body;
    std::string m_slot_name;
    std::vector<Param> m_slot_params;
    bool m_slot_open{false};
    bool m_vtable_open{false};
};

} // namespace cc_abi_gen::classify

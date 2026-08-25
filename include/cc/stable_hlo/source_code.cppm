module;

export module cc_stable_hlo:source_code;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace cc::stable_hlo {

// Concrete in-process rendering sink — the ice::GeneratorSourceCodeBase implementation every
// IR node's render_into() writes through.
class StableHloSourceCode : public ice::GeneratorSourceCodeBase {
  public:
    void add_line(std::string_view line) override {

        if (!line.empty()) {
            m_buffer.append(static_cast<std::size_t>(m_indent_depth * m_spaces_per_indent), ' ');
            m_buffer += line;
        }
        m_buffer += '\n';

    }
    void add_blank_line() override { m_buffer += '\n'; }
    void increase_indent() override { ++m_indent_depth; }
    void decrease_indent() override {

        if (m_indent_depth > 0) --m_indent_depth;

    }
    void set_spaces_per_indent(int spaces) override { m_spaces_per_indent = spaces; }
    ice::StringBuilder render() const override { return ice::StringBuilder{m_buffer}; }

  private:
    std::string m_buffer;
    int m_indent_depth{0};
    int m_spaces_per_indent{4};
};

} // namespace cc::stable_hlo

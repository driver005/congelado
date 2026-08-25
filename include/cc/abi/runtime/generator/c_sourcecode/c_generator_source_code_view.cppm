module;

#include "c/extern/generator/sourcecode.h"

export module cc_abi_runtime_generator:c_generator_source_code_view;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace ice {

// Non-owning view
class CGeneratorSourceCodeView : public GeneratorSourceCodeBase {
public:
    CGeneratorSourceCodeView() : m_handle(nullptr) {}
    explicit CGeneratorSourceCodeView(TF_Generator_SourceCode* handle) : m_handle(handle) {}
    ~CGeneratorSourceCodeView() override = default;

    CGeneratorSourceCodeView(const CGeneratorSourceCodeView&) = default;
    CGeneratorSourceCodeView& operator=(const CGeneratorSourceCodeView&) = default;
    CGeneratorSourceCodeView(CGeneratorSourceCodeView&&) = default;
    CGeneratorSourceCodeView& operator=(CGeneratorSourceCodeView&&) = default;

    void add_line(std::string_view line) override {

        if (!m_handle) return;
        std::string null_terminated{line};
        TF_Generator_SourceCode_AddLine(m_handle, null_terminated.c_str());

    }
    void add_blank_line() override {

        if (!m_handle) return;
        TF_Generator_SourceCode_AddBlankLine(m_handle);

    }
    void increase_indent() override {

        if (!m_handle) return;
        TF_Generator_SourceCode_IncreaseIndent(m_handle);

    }
    void decrease_indent() override {

        if (!m_handle) return;
        TF_Generator_SourceCode_DecreaseIndent(m_handle);

    }
    void set_spaces_per_indent(int spaces) override {

        if (!m_handle) return;
        TF_Generator_SourceCode_SetSpacesPerIndent(m_handle, spaces);

    }

    StringBuilder render() const override {

        if (!m_handle) return StringBuilder{};
        char* rendered = TF_Generator_SourceCode_Render(m_handle);
        StringBuilder result(rendered);
        TF_Generator_SourceCode_FreeRendered(rendered);
        return result;

    }

    TF_Generator_SourceCode* get_handle() { return m_handle; }
    const TF_Generator_SourceCode* get_handle() const { return m_handle; }

private:
    TF_Generator_SourceCode* m_handle;
};

} // namespace ice

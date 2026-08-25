module;

#include "c/extern/generator/sourcecode.h"

export module cc_abi_runtime_generator:c_generator_source_code;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;

export namespace ice {

// C ABI adapter: implements GeneratorSourceCodeBase by calling TF_Generator_SourceCode_* functions.
class CGeneratorSourceCode : public GeneratorSourceCodeBase {
public:
    CGeneratorSourceCode() : m_handle(TF_Generator_SourceCode_Create()) {}
    explicit CGeneratorSourceCode(TF_Generator_SourceCode* handle) : m_handle(handle) {}
    ~CGeneratorSourceCode() override {

        if (m_handle) TF_Generator_SourceCode_Destroy(m_handle);

    }

    CGeneratorSourceCode(const CGeneratorSourceCode&) = delete;
    CGeneratorSourceCode& operator=(const CGeneratorSourceCode&) = delete;

    CGeneratorSourceCode(CGeneratorSourceCode&& other) noexcept : m_handle(other.m_handle) {

        other.m_handle = nullptr;

    }
    CGeneratorSourceCode& operator=(CGeneratorSourceCode&& other) noexcept {

        if (this != &other) {
            if (m_handle) TF_Generator_SourceCode_Destroy(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    void add_line(std::string_view line) override {

        std::string null_terminated{line};
        TF_Generator_SourceCode_AddLine(m_handle, null_terminated.c_str());

    }
    void add_blank_line() override {

        TF_Generator_SourceCode_AddBlankLine(m_handle);

    }
    void increase_indent() override {

        TF_Generator_SourceCode_IncreaseIndent(m_handle);

    }
    void decrease_indent() override {

        TF_Generator_SourceCode_DecreaseIndent(m_handle);

    }
    void set_spaces_per_indent(int spaces) override {

        TF_Generator_SourceCode_SetSpacesPerIndent(m_handle, spaces);

    }

    StringBuilder render() const override {

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

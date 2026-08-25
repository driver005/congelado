module;

#include "c/extern/generator/controller.h"
#include "c/extern/generator/sourcecode.h"

export module cc_abi_runtime_generator:c_controller;

import std;
import cc_abi_builder_intern;
import cc_abi_builder_generator;
import :c_definition;
import :c_sourcecode;

export namespace ice {

// GeneratorRuntime — the mainframe-facing generator handle. Unifies the two ways a named
// generator can be reached:
//   - in-process: a GeneratorBuilderBase registered directly into GeneratorBuilderRegistry
//     (e.g. "stablehlo" — see cc_stable_hlo's generator.cppm), looked up and held non-owning.
//   - cross-plugin: no in-process registrant for that name, so this falls back to the raw
//     TF_Generator_* C ABI (TF_Generator_Create), owning the resulting handle and forwarding
//     every call across it — the same role today's CGeneratorController played, just reached
//     through the same name-based create() instead of a separate always-C-ABI path.
// Either way, the mainframe calls GeneratorRuntime::create("stablehlo", ...) and doesn't need
// to know which case it got.
class GeneratorRuntime : public GeneratorBuilderBase {
public:
    ~GeneratorRuntime() override {
        if (m_c_handle) TF_Generator_Destroy(m_c_handle);
    }

    GeneratorRuntime(const GeneratorRuntime&) = delete;
    GeneratorRuntime& operator=(const GeneratorRuntime&) = delete;
    GeneratorRuntime(GeneratorRuntime&&) = delete;
    GeneratorRuntime& operator=(GeneratorRuntime&&) = delete;

    static std::unique_ptr<GeneratorRuntime> create(std::string_view name, std::string_view output_dir,
                                                     std::string_view source_dir, TF_Status* status) {
        GeneratorBuilderBase* in_process =
            GeneratorBuilderRegistry::default_registry().get_or_create(name, output_dir, source_dir);
        if (in_process) {
            return std::unique_ptr<GeneratorRuntime>(new GeneratorRuntime(in_process));
        }

        TF_Generator_Controller* handle = TF_Generator_Create(std::string{output_dir}.c_str(),
                                                               std::string{source_dir}.c_str(), status);
        if (TF_GetCode(status) != TF_OK) {
            if (handle) TF_Generator_Destroy(handle);
            return nullptr;
        }
        return std::unique_ptr<GeneratorRuntime>(new GeneratorRuntime(handle));
    }

    void write_file(std::string_view path, const GeneratorSourceCodeBase& code) override {
        if (m_in_process) {
            m_in_process->write_file(path, code);
            return;
        }
        const auto* c_code = dynamic_cast<const CGeneratorSourceCode*>(&code);
        if (!c_code) return;
        TF_Status* status = TF_NewStatus();
        TF_Generator_WriteFile(m_c_handle, std::string{path}.c_str(), c_code->get_handle(), status);
        TF_DeleteStatus(status);
    }

    void write_module(std::string_view path) override {
        if (m_in_process) {
            m_in_process->write_module(path);
            return;
        }
        TF_Status* status = TF_NewStatus();
        TF_Generator_WriteModule(m_c_handle, std::string{path}.c_str(), status);
        TF_DeleteStatus(status);
    }

    size_t get_definition_count() const override {
        if (m_in_process) return m_in_process->get_definition_count();
        return TF_Generator_GetDefinitionCount(m_c_handle);
    }

    std::unique_ptr<GeneratorDefinitionViewBase> get_definition(size_t index) const override {
        if (m_in_process) return m_in_process->get_definition(index);
        const TF_Generator_Definition* def = TF_Generator_GetDefinition(m_c_handle, index);
        if (!def) return nullptr;
        return std::make_unique<CGeneratorDefinitionView>(def);
    }

    void set_name(std::string_view name) override {
        if (m_in_process) {
            m_in_process->set_name(name);
            return;
        }
        TF_String tf_name;
        TF_StringInit(&tf_name);
        TF_StringAssignView(&tf_name, name.data(), name.size());
        TF_Generator_SetName(m_c_handle, &tf_name);
        TF_StringDealloc(&tf_name);
    }

    StringBuilder get_name() const override {
        if (m_in_process) return m_in_process->get_name();
        TF_String tf_name;
        TF_StringInit(&tf_name);
        TF_Generator_GetName(m_c_handle, &tf_name);
        StringBuilder result{&tf_name};
        TF_StringDealloc(&tf_name);
        return result;
    }

    // True if this generator was resolved in-process (no C-ABI crossing).
    bool is_in_process() const { return m_in_process != nullptr; }

    // Underlying handle — only meaningful for the cross-plugin path
    TF_Generator_Controller* get_handle() { return m_c_handle; }
    const TF_Generator_Controller* get_handle() const { return m_c_handle; }

private:
    explicit GeneratorRuntime(GeneratorBuilderBase* in_process) : m_in_process(in_process), m_c_handle(nullptr) {}
    explicit GeneratorRuntime(TF_Generator_Controller* c_handle) : m_in_process(nullptr), m_c_handle(c_handle) {}

    GeneratorBuilderBase* m_in_process;       // non-owning — GeneratorBuilderRegistry owns it
    TF_Generator_Controller* m_c_handle;      // owning — this object destroys it
};

} // namespace ice

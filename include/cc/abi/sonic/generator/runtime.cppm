module;

#include "c/extern/generator/controller.h"

export module cc_abi_sonic_generator:controller;

import std;
import cc_abi_sonic_intern;
import cc_abi_builder_generator;
import :definition;
import :function;

export namespace ice::sonic {

// Runtime — the mainframe-facing generator handle. Unifies the two ways a named
// generator can be reached:
//   - in-process: an ice::builder::generator::Builder registered directly into
//     ice::builder::generator::BuilderRegistry (e.g. "stablehlo" — see cc_stable_hlo's
//     generator.cppm), looked up and held non-owning.
//   - cross-plugin: no in-process registrant for that name, so this falls back to the raw
//     TF_Generator_* C ABI (TF_Generator_Create), owning the resulting handle and forwarding
//     every call across it.
// Either way, the mainframe calls Runtime::create("stablehlo", ...) and doesn't need
// to know which case it got.
class Runtime : public ice::builder::generator::Builder
{
public:
    ~Runtime() override = default; // m_c_handle's own deleter runs TF_Generator_Destroy

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    static std::unique_ptr<Runtime> create(
        std::string_view name,
        std::string_view output_dir,
        std::string_view source_dir,
        TF_Status* status
    )
    {

        ice::builder::generator::Builder* in_process =
            ice::builder::generator::BuilderRegistry::default_registry().get_or_create(
                name, output_dir, source_dir
            );
        if (in_process) {
            return std::unique_ptr<Runtime>(new Runtime(*in_process));
        }

        TF_Generator_Controller* handle = TF_Generator_Create(
            std::string{output_dir}.c_str(), std::string{source_dir}.c_str(), status
        );
        if (TF_GetCode(status) != TF_OK) {
            if (handle) {
                TF_Generator_Destroy(handle);
            }
            return nullptr;
        }
        return std::unique_ptr<Runtime>(new Runtime(handle));
    }

    size_t get_definition_count() const override
    {

        if (m_in_process) {
            return m_in_process->get().get_definition_count();
        }
        return TF_Generator_GetDefinitionCount(m_c_handle.get());
    }

    std::unique_ptr<ice::builder::generator::Definition>
    get_definition(size_t index) const override
    {

        if (m_in_process) {
            return m_in_process->get().get_definition(index);
        }
        const TF_Generator_Definition* def = TF_Generator_GetDefinition(m_c_handle.get(), index);
        if (!def) {
            return nullptr;
        }
        return std::make_unique<Definition>(def);
    }

    void set_name(std::string_view name) override
    {

        if (m_in_process) {
            m_in_process->get().set_name(name);
            return;
        }
        TF_String tf_name;
        TF_StringInit(&tf_name);
        TF_StringAssignView(&tf_name, name.data(), name.size());
        TF_Generator_SetName(m_c_handle.get(), &tf_name);
        TF_StringDealloc(&tf_name);
    }

    ice::sonic::StringRuntime get_name() const override
    {

        if (m_in_process) {
            return m_in_process->get().get_name();
        }
        TF_String tf_name;
        TF_StringInit(&tf_name);
        TF_Generator_GetName(m_c_handle.get(), &tf_name);
        ice::sonic::StringRuntime result{&tf_name};
        TF_StringDealloc(&tf_name);
        return result;
    }

    std::expected<ice::sonic::StringRuntime, ice::sonic::StringRuntime> build() const override
    {

        if (m_in_process) {
            return m_in_process->get().build();
        }
        TF_Status* status = TF_NewStatus();
        TF_String out;
        TF_StringInit(&out);
        bool ok = TF_Generator_Build(m_c_handle.get(), &out, status);
        if (!ok) {
            ice::sonic::StringRuntime message{std::string{TF_Message(status)}};
            TF_StringDealloc(&out);
            TF_DeleteStatus(status);
            return std::unexpected{message};
        }
        ice::sonic::StringRuntime result{&out};
        TF_StringDealloc(&out);
        TF_DeleteStatus(status);
        return result;
    }

    // --- generic construction path — see ice::builder::generator::Builder's class comment ---

    std::expected<std::reference_wrapper<ice::builder::generator::Function>, ice::sonic::StringRuntime>
    enter_border_patrol(std::string_view name) override
    {

        if (m_in_process) {
            return m_in_process->get().enter_border_patrol(name);
        }
        TF_Status* status = TF_NewStatus();
        TF_Generator_Function* handle =
            TF_Generator_EnterBorderPatrol(m_c_handle.get(), std::string{name}.c_str(), status);
        if (!handle) {
            ice::sonic::StringRuntime message{std::string{TF_Message(status)}};
            TF_DeleteStatus(status);
            return std::unexpected{message};
        }
        TF_DeleteStatus(status);
        m_open_function.emplace(handle);
        return *m_open_function;
    }

    // True if this generator was resolved in-process (no C-ABI crossing).
    bool is_in_process() const
    {
        return m_in_process.has_value();
    }

    // Underlying handle — only meaningful for the cross-plugin path
    TF_Generator_Controller* get_handle()
    {
        return m_c_handle.get();
    }

    const TF_Generator_Controller* get_handle() const
    {
        return m_c_handle.get();
    }

private:
    // Stateless — m_c_handle's deleter, so ~Runtime() doesn't need a manual
    // "if (m_c_handle) TF_Generator_Destroy(...)" (unique_ptr already skips a null deleter call).
    struct ControllerDeleter
    {
        void operator()(TF_Generator_Controller* handle) const
        {
            TF_Generator_Destroy(handle);
        }
    };

    explicit Runtime(ice::builder::generator::Builder& in_process) :
        m_in_process{in_process}
    {
    }

    explicit Runtime(TF_Generator_Controller* c_handle) :
        m_c_handle{c_handle}
    {
    }

    // non-owning — ice::builder::generator::BuilderRegistry owns the referenced Builder.
    std::optional<std::reference_wrapper<ice::builder::generator::Builder>> m_in_process;
    // owning — RAII via ControllerDeleter instead of a raw pointer + manual destructor.
    std::unique_ptr<TF_Generator_Controller, ControllerDeleter> m_c_handle;
    // Owns whichever construction unit is currently open (cross-plugin path only — the
    // in-process path's own Builder owns its own equivalent state instead, see
    // cc::stable_hlo::Builder::m_function_handle). ice::builder::generator::Function itself is
    // abstract (can't be a plain value member) — Function here is this file's own concrete
    // adapter, owned by value, no heap allocation. emplace() destroys whatever was previously
    // open (running ~Function()'s TF_Generator_Function_Destroy) before constructing the new
    // one — same "a fresh call replaces the current one" lifetime story as everything else on
    // this interface.
    std::optional<Function> m_open_function;
};

} // namespace ice::sonic

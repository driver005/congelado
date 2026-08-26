module;

export module cc_abi_builder_generator:generator_builder;

import std;
import cc_abi_sonic_intern;
import :definition;
import :generator_function;

export namespace ice::builder::generator {

// Abstract base class for a generator builder — implemented by generator implementations
// (e.g. stable_hlo) AND by the C ABI adapter (sonic/generator's cross-plugin path).
//
// Two-tier construction design: a generator implementation is free to expose its own typed,
// native construction API (e.g. cc::stable_hlo::Builder::add_op, which knows about opcodes and
// shapes) for convenient same-binary C++ callers — that native API never needs to cross the
// C ABI. Alongside it, enter_border_patrol() gives every generator implementation an OPTIONAL
// second construction path that IS crossable through the C ABI
// (TF_Generator_EnterBorderPatrol), because it carries no generator-specific vocabulary —
// "node" instead of "op", plain string kinds/attrs instead of typed ones. Named after crossing
// a checkpoint at the C ABI border — enter_border_patrol() opens a new named construction unit
// (function/basic block/`def` body/whatever the target calls it) and gets waved through,
// returning a generator::Function handle to build it with (add_parameter()/add_node()/
// exit_border_patrol() — see that class) — since "scope"/"function"/"block" all already mean
// something more specific in most target languages, and this needed a name that means nothing
// in any of them. A generator that has no notion of construction (read-only/catalog-only)
// simply doesn't override this; the default reports "unsupported".
class Builder
{
public:
    virtual ~Builder() = default;

    // Factory method — implemented by each generator
    using Factory = std::unique_ptr<Builder> (*)(
        std::string_view output_dir, std::string_view source_dir
    );

    // Get number of op definitions
    virtual std::size_t get_definition_count() const = 0;

    // Get definition view by index
    virtual std::unique_ptr<Definition> get_definition(std::size_t index) const = 0;

    // Generator identity
    virtual void set_name(std::string_view name) = 0;
    virtual ice::sonic::StringRuntime get_name() const = 0;

    // Renders whatever this generator holds (e.g. the modules a caller appended) into text.
    // Error channel is a StringRuntime message, not a generator-specific error type — this
    // interface can't depend on any one implementation's own error type (e.g. cc::stable_hlo::
    // Error), same reason enter_border_patrol()'s error channel below is a StringRuntime too.
    virtual std::expected<ice::sonic::StringRuntime, ice::sonic::StringRuntime> build() const = 0;

    // --- generic construction path — optional, see class comment ---

    // Opens a new named construction unit. The implementation owns it (a fresh call replaces
    // whatever was previously open — same lifetime story as everything else on this
    // interface); this hands back a reference to build it with — see generator::Function. An
    // error message if this generator doesn't support construction.
    virtual std::expected<std::reference_wrapper<Function>, ice::sonic::StringRuntime>
    enter_border_patrol(std::string_view)
    {
        return std::unexpected{ice::sonic::StringRuntime{std::string{"unsupported"}}};
    }
};

} // namespace ice::builder::generator

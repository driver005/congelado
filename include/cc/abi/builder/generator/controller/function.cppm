module;

export module cc_abi_builder_generator:generator_function;

import std;
import cc_abi_sonic_intern;
import :node_handle;

export namespace ice::builder::generator {

// One open construction unit (function/basic block/`def` body/whatever the target calls it),
// returned by Builder::enter_border_patrol() — add_parameter()/add_node() build it up, then
// exit_border_patrol() closes it. Carries no generator-specific vocabulary (see Builder's own
// class comment), so it crosses the C ABI the same way Builder's other generic methods do.
class Function
{
public:
    virtual ~Function() = default;

    // Adds one input parameter, named `name`, typed by `type_text` (opaque to this interface —
    // the target generator's own textual type syntax, e.g. for stablehlo "4xf32"/"f32", see
    // cc_stable_hlo's Shape::parse). The new parameter's handle on success; an error message
    // (type_text didn't parse) on failure.
    virtual std::expected<NodeHandle, ice::sonic::StringRuntime>
    add_parameter(std::string_view name, std::string_view type_text) = 0;

    // Adds one construct — an op/instruction/statement/whatever the target calls its atomic
    // unit — identified by a plain string `kind`, taking `operands` and `attrs` (generic string
    // key/value pairs). Results are written into `out_results` (caller-allocated, sized to the
    // wanted result count). Returns false on failure.
    virtual bool add_node(
        std::string_view kind,
        std::span<const NodeHandle> operands,
        std::span<const std::pair<std::string_view, std::string_view>> attrs,
        std::span<NodeHandle> out_results
    ) = 0;

    // Closes this unit, marking `outputs` as its outputs. Returns false on failure.
    virtual bool exit_border_patrol(std::span<const NodeHandle> outputs) = 0;
};

} // namespace ice::builder::generator

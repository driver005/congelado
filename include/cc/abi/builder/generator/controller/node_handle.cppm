module;

export module cc_abi_builder_generator:node_handle;

import std;

export namespace ice::builder::generator {

// A generic, language-agnostic handle to one constructed value — an SSA id, an AST node
// pointer, a register number, whatever the target generator uses internally. Meaningless
// outside the Builder instance that produced it; crosses the C ABI as a plain size_t
// (see TF_Generator_Function_AddNode et al. in c/extern/generator/controller.h).
struct NodeHandle
{
    std::size_t id{};
};

} // namespace ice::builder::generator

module;

export module cc_abi_builder_generator;

// Abstract base classes (building blocks for generator implementations) — pure interface,
// zero StableHLO/plugin-specific knowledge. A generator implementation (e.g. stable_hlo)
// implements these directly and registers itself into ice::builder::generator::BuilderRegistry; this module
// never imports any specific generator implementation, which is what makes the dependency
// one-way (StableHLO sits above this layer, not the reverse).
export import :node_handle;
export import :generator_function;
export import :generator_builder;
export import :definition;
export import :parameter;
export import :typeinfo;
export import :attribute;

// Registry and factory
export import :registry;

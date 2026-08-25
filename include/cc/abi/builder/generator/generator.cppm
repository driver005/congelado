module;

export module cc_abi_builder_generator;

// Abstract base classes (building blocks for generator implementations) — pure interface,
// zero StableHLO/plugin-specific knowledge. A generator implementation (e.g. stable_hlo)
// implements these directly and registers itself into GeneratorBuilderRegistry; this module
// never imports any specific generator implementation, which is what makes the dependency
// one-way (StableHLO sits above this layer, not the reverse).
export import :generator_builder_base;
export import :generator_builder_view_base;
export import :base_definition;
export import :base_parameter;
export import :base_typeinfo;
export import :base_attribute;
export import :base_sourcecode;

// Registry and factory
export import :registry;

module;

export module cc_abi_sonic_generator;

// Standalone C ABI adapter classes that cross through the flat TF_Generator
// vtable from c/extern/generator/generator.h.
export import :attribute;
export import :definition;
export import :function;
export import :parameter;
export import :typeinfo;

// The unified mainframe-facing handle — resolved through ice::sonic::RegistrationRuntime
// (type="generator") via init_generator, one API.
export import :runtime;

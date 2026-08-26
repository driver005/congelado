module;

export module cc_abi_sonic_generator;

import cc_abi_builder_generator;

// C ABI adapter classes (implement ice::builder::generator::* base classes via TF_Generator_*)
export import :definition;
export import :parameter;
export import :typeinfo;
export import :attribute;
export import :function;

// The unified mainframe-facing handle — in-process or cross-plugin, one API.
export import :controller;

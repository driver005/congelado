module;

export module cc_abi_runtime_generator;

import cc_abi_builder_generator;

// C ABI adapter classes (implement builder base classes via TF_Generator_*)
export import :c_definition;
export import :c_parameter;
export import :c_typeinfo;
export import :c_attribute;
export import :c_sourcecode;

// The unified mainframe-facing handle — in-process or cross-plugin, one API.
export import :c_controller;

export module cc_abi_gen_classify:utils;

import std;

export namespace cc_abi_gen::classify {

enum class FunctionType : std::uint8_t
{
    STANDARD,
    CONSTRUCTOR,
    COPY_CONSTRUCTOR,
    MOVE_CONSTRUCTOR,
    DESTRUCTOR,
    // `operator T()`: no return type in front of the name.
    CONVERSION,
};

// What C declaration a class was parsed from.
enum class Kind : std::uint8_t
{
    VTABLE,
    STRUCT,
    ENUM,
};

} // namespace cc_abi_gen::classify

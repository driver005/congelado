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
};

} // namespace cc_abi_gen::classify

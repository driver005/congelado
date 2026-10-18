// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/option_types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/option_types.h"

export module cc_ice_extern_filesystem_sonic:filesystem_option_type;

import std;

export namespace ice::sonic {

class FilesystemOptionType
{
public:
    enum Value : std::underlying_type_t<::TFFilesystemOptionType>
    {
        k_int = ::TF_FILESYSTEM_OPTION_TYPE_INT,
        k_real = ::TF_FILESYSTEM_OPTION_TYPE_REAL,
        k_buffer = ::TF_FILESYSTEM_OPTION_TYPE_BUFFER
    };

    constexpr FilesystemOptionType() noexcept = default;

    constexpr FilesystemOptionType(::TFFilesystemOptionType value) noexcept :
        m_value{value}
    {
    }

    constexpr FilesystemOptionType(Value value) noexcept :
        m_value{static_cast<::TFFilesystemOptionType>(value)}
    {
    }

    constexpr ::TFFilesystemOptionType get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFFilesystemOptionType() const noexcept
    {
        return m_value;
    }

private:
    ::TFFilesystemOptionType m_value{};
};

} // namespace ice::sonic

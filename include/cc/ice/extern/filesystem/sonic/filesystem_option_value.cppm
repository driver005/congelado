// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/option_types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/option_types.h"

export module cc_ice_extern_filesystem_sonic:filesystem_option_value;

import std;

export namespace ice::sonic {

class FilesystemOptionValue
{
public:
    FilesystemOptionValue() noexcept {}

    explicit FilesystemOptionValue(const ::TFFilesystemOptionValue& value) noexcept :
        m_value{value}
    {
    }

    explicit FilesystemOptionValue(::TFFilesystemOptionType type) noexcept
    {
        m_value.type = type;
    }

    ::TFFilesystemOptionType getType() const noexcept
    {
        return m_value.type;
    }

    void setType(::TFFilesystemOptionType value) noexcept
    {
        m_value.type = value;
    }

    ::TFFilesystemOptionValue* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TFFilesystemOptionValue* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TFFilesystemOptionValue m_value{};
};

} // namespace ice::sonic

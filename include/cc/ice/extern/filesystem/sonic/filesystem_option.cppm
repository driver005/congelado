// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/option_types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/option_types.h"

export module cc_ice_extern_filesystem_sonic:filesystem_option;

import std;

export namespace ice::sonic {

class FilesystemOption
{
public:
    FilesystemOption() noexcept {}

    explicit FilesystemOption(const ::TFFilesystemOption& value) noexcept :
        m_value{value}
    {
    }

    ::TFFilesystemOptionType getTypeTag() const noexcept
    {
        return m_value.type_tag;
    }

    void setTypeTag(::TFFilesystemOptionType value) noexcept
    {
        m_value.type_tag = value;
    }

    const char* getName() const noexcept
    {
        return m_value.name;
    }

    void setName(const char* value) noexcept
    {
        m_value.name = value;
    }

    const char* getDescription() const noexcept
    {
        return m_value.description;
    }

    void setDescription(const char* value) noexcept
    {
        m_value.description = value;
    }

    ::TFFilesystemOptionValue& getValue() noexcept
    {
        return m_value.value;
    }

    const ::TFFilesystemOptionValue& getValue() const noexcept
    {
        return m_value.value;
    }

    ::TFFilesystemOption* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TFFilesystemOption* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TFFilesystemOption m_value{};
};

} // namespace ice::sonic

module;

#include "c/extern/filesystem/option_types.h"

export module cc_abi_primitives:filesystem_option;

import std;

export namespace ice {

// FilesystemOption — thin value wrapper over the C TF_Filesystem_Option struct, so
// option values cross by value without leaking the raw struct. Every member is
// noexcept; the wrapped struct is trivially copyable, so the defaulted copy ctor /
// assignment are correct.
class FilesystemOption
{
public:
    FilesystemOption() noexcept = default;

    explicit FilesystemOption(const TF_Filesystem_Option& option) noexcept :
        m_option{option}
    {
    }

    TF_Filesystem_Option to_c() const noexcept
    {
        return m_option;
    }

    static FilesystemOption create(const TF_Filesystem_Option& option) noexcept
    {
        return FilesystemOption{option};
    }

    TF_Filesystem_Option_Type get_type_tag() const noexcept
    {
        return m_option.type_tag;
    }

    const char* get_name() const noexcept
    {
        return m_option.name;
    }

    const char* get_description() const noexcept
    {
        return m_option.description;
    }

    TF_Filesystem_Option_Value get_value() const noexcept
    {
        return m_option.value;
    }

    const TF_Filesystem_Option* get_handle() const noexcept
    {
        return &m_option;
    }

    TF_Filesystem_Option* get_handle() noexcept
    {
        return &m_option;
    }

private:
    TF_Filesystem_Option m_option{};
};

// delete_recursively's out-params, collected into one value the caller passes in.
struct DeleteRecursivelyResult
{
    std::uint64_t undeleted_files;
    std::uint64_t undeleted_dirs;
};

} // namespace ice

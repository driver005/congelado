module;

export module cc_abi_value:filesystem_option;

import std;

export namespace ice {

// Option struct wrapper
struct FilesystemOption
{
    std::string name;
    std::string description;
    bool per_file{};
    // Value is owned by caller
};

} // namespace ice

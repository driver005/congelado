module;

#include "c/extern/filesystem.h"

export module cc_abi_value:filesystem_option_value;

import std;

export namespace ice {

// Option type enum
enum class FilesystemOptionType {
    Int = TF_Filesystem_Option_Type_Int,
    Real = TF_Filesystem_Option_Type_Real,
    Buffer = TF_Filesystem_Option_Type_Buffer,
};

// Option value union wrapper
struct FilesystemOptionValue {
    FilesystemOptionType type_tag{};
    int num_values{};
    // Values are owned by caller, managed externally
    // This mirrors the C TF_Filesystem_Option_Value_Union
};

} // namespace ice

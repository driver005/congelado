module;

#include "c/extern/env/dynamic_library.h"

export module cc_abi_builder_env:dynamic_library;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class DynamicLibrary
{
public:
    [[nodiscard]] static std::expected<void*, ice::Status>
    load(const ice::String& library_filename)
    {

        ice::Status status;
        void* handle = TF_LoadSharedLibrary(library_filename.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return handle;
    }

    [[nodiscard]] static std::expected<void*, ice::Status>
    get_symbol(void* handle, const ice::String& symbol_name)
    {

        ice::Status status;
        void* symbol = TF_GetSymbolFromLibrary(handle, symbol_name.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return symbol;
    }
};

} // namespace ice::builder

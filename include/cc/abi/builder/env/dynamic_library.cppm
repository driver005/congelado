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
    [[nodiscard]] static std::expected<void*, ice::Status> load(const ice::String& library_filename) noexcept
    {
        ice::Status status;
        void* handle = load_shared_library(library_filename.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return handle;
    }

    [[nodiscard]] static std::expected<void*, ice::Status>
    get_symbol(void* handle, const ice::String& symbol_name) noexcept
    {
        ice::Status status;
        void* symbol = get_symbol_from_library(handle, symbol_name.c_str(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return symbol;
    }
};

} // namespace ice::builder

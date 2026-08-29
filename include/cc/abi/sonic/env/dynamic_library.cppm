module;

#include <dlfcn.h>

export module cc_abi_sonic_env:dynamic_library;

import std;
import cc_abi_sonic_intern;
import :symbol;

export namespace ice::sonic {

// Real POSIX dlopen/dlsym-backed dynamic library loader — include/c/ is declaration-only, so
// this doesn't forward to c/extern/env/dynamic_library.h's load_shared_library/
// get_symbol_from_library (which have no implementation anywhere and never will); it calls
// dlopen/dlsym directly instead, same RTLD_NOW | RTLD_LOCAL choice
// include/core/manager/shared_lib.cppm's own real plugin loader already uses. Owns the loaded
// handle (RAII dlclose), move-only.
class DynamicLibrary
{
public:
    DynamicLibrary() = default;

    ~DynamicLibrary()
    {
        if (m_lib_handle) {
            dlclose(m_lib_handle);
        }
    }

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&& other) noexcept :
        m_lib_handle{other.m_lib_handle}
    {
        other.m_lib_handle = nullptr;
    }

    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept
    {
        if (this != &other) {
            if (m_lib_handle) {
                dlclose(m_lib_handle);
            }
            m_lib_handle = other.m_lib_handle;
            other.m_lib_handle = nullptr;
        }
        return *this;
    }

    [[nodiscard]] std::expected<void, Status> on_load(const String& library_filename)
    {
        void* handle = dlopen(library_filename.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            return std::unexpected{Status{dlerror()}};
        }
        m_lib_handle = handle;
        return {};
    }

    [[nodiscard]] std::expected<Symbol, Status> get_symbol(const String& symbol_name) const
    {
        dlerror(); // clear any pending error — dlsym's own success can't otherwise be told
                   // apart from a symbol that's legitimately bound to NULL (POSIX dlsym
                   // contract)
        void* symbol = dlsym(m_lib_handle, symbol_name.c_str());
        const char* error = dlerror();
        if (error) {
            return std::unexpected{Status{error}};
        }
        return Symbol{symbol};
    }

private:
    void* m_lib_handle{nullptr};
};

} // namespace ice::sonic

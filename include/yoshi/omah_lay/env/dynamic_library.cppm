module;

#include <dlfcn.h>

export module yoshi_omah_lay_env:dynamic_library;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;

export namespace cc::extension {

// Real POSIX dlopen/dlsym-backed dynamic library loader — same RTLD_NOW | RTLD_LOCAL choice
// include/core/manager/shared_lib.cppm's own plugin loader already uses (immediate symbol
// resolution, no global-namespace pollution across plugins). Errors surface as
// ice::Status, same idiom cc::stable_hlo/ice::sonic::generator use throughout — this
// bypasses the c/extern/env/dynamic_library.h TF_LoadSharedLibrary/TF_GetSymbolFromLibrary
// C ABI shim entirely rather than implementing it, since nothing here needs to cross the C ABI.
class DynamicLibrary
{
public:
    DynamicLibrary() = default;

    ~DynamicLibrary()
    {

        if (m_handle) {
            dlclose(m_handle);
        }
    }

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept
    {

        if (this != &other) {
            if (m_handle) {
                dlclose(m_handle);
            }
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    [[nodiscard]] std::expected<void, ice::Status> load(const std::string& library_filename)
    {

        if (m_handle) {
            dlclose(m_handle);
            m_handle = nullptr;
        }
        m_handle = dlopen(library_filename.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!m_handle) {
            return std::unexpected{ice::Status{dlerror()}};
        }
        return {};
    }

    [[nodiscard]] std::expected<void*, ice::Status>
    get_symbol(const std::string& symbol_name) const
    {

        dlerror(); // clear any pending error — dlsym's own success can't otherwise be told
                   // apart from a symbol that's legitimately bound to NULL (POSIX dlsym
                   // contract)
        void* symbol = dlsym(m_handle, symbol_name.c_str());
        const char* error = dlerror();
        if (error) {
            return std::unexpected{ice::Status{error}};
        }
        return symbol;
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_handle != nullptr;
    }

private:
    void* m_handle{nullptr};
};

} // namespace cc::extension

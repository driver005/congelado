module;

#include <dlfcn.h>

export module aten_xpu_profiler_common:dynamic_library;

import std;

export namespace aten_xpu {

// A vendor runtime loaded at run time, so the plugin has no link dependency on any of them.
class DynamicLibrary
{
public:
    DynamicLibrary() = default;

    ~DynamicLibrary()
    {
        if (m_handle != nullptr) {
            dlclose(m_handle);
        }
    }

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;
    DynamicLibrary(DynamicLibrary&&) = delete;
    DynamicLibrary& operator=(DynamicLibrary&&) = delete;

    // First name that loads wins. `only_loaded` never pulls a runtime into the process: it
    // succeeds only for a library that something else already loaded.
    bool open(std::span<const std::string_view> names, bool only_loaded)
    {
        if (m_handle != nullptr) {
            return true;
        }

        const int FLAGS = RTLD_LAZY | (only_loaded ? RTLD_NOLOAD : 0);
        for (const std::string_view name: names) {
            m_handle = dlopen(std::string{name}.c_str(), FLAGS);
            if (m_handle != nullptr) {
                return true;
            }
        }

        return false;
    }

    template<typename Function>
    [[nodiscard]] Function symbol(const char* name) const
    {
        return reinterpret_cast<Function>(dlsym(m_handle, name));
    }

private:
    void* m_handle{nullptr};
};

} // namespace aten_xpu

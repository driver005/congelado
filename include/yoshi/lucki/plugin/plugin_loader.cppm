module;

export module yoshi_lucki_plugin:plugin_loader;

import std;
import cc_abi_sonic_env;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_plugin;

export namespace yoshi::core {

// Loads plugin shared libraries and calls each one's TF_InitPlugin — same fixed-entry-point,
// no-callback-into-host contract TensorFlow's real filesystem plugins use, applied to the
// generic TF_PluginInfo (c/extern/plugin/registration.h) rather than the filesystem-specific
// one. Never names a TF_* symbol/type/header directly — only calls into ice::sonic::* wrapper
// types (DynamicLibraryRuntime, Symbol, PluginRuntime), which own that boundary. Every
// successfully loaded library is kept in m_lib_handles for this PluginLoader's own lifetime —
// DynamicLibraryRuntime dlcloses on destruction, so without this a loaded plugin (and anything
// it registered a pointer to elsewhere, e.g. ice::sonic::RegistrationRuntime) would dangle the
// instant load() returned.
class PluginLoader
{
public:
    ~PluginLoader()
    {
        close_all();
    }

    [[nodiscard]] std::expected<ice::sonic::PluginRuntime, ice::Status>
    load(const std::string& path)
    {

        ice::sonic::DynamicLibraryRuntime library;
        auto loaded = library.on_load(ice::String{path});
        if (!loaded) {
            return std::unexpected{loaded.error()};
        }

        auto symbol = library.get_symbol(
            ice::String{std::string{"TF_InitPlugin"}}
        );
        if (!symbol) {
            return std::unexpected{symbol.error()};
        }

        auto plugin_info = ice::sonic::PluginRuntime::create();
        symbol->call(plugin_info.get_handle());
        m_lib_handles.emplace(path, std::move(library));
        return plugin_info;
    }

    // Closes one loaded plugin by the path it was load()ed with — erasing the map entry
    // destroys its DynamicLibraryRuntime, which dlcloses.
    void close(const std::string& path)
    {
        m_lib_handles.erase(path);
    }

    // Closes every loaded plugin.
    void close_all()
    {
        m_lib_handles.clear();
    }

private:
    std::unordered_map<std::string, ice::sonic::DynamicLibraryRuntime> m_lib_handles;
};

} // namespace yoshi::core

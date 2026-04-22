module;

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

export module core_plugin:loader;

import std;
import :handle;

export namespace core::plugin {

class DynamicLibrary {
    void *handle_ = nullptr;

  public:
    DynamicLibrary() = default;

    explicit DynamicLibrary(const std::filesystem::path &path) {
        auto path_str = path.string();
#if defined(_WIN32)
        handle_ = static_cast<void *>(LoadLibraryA(path_str.c_str()));
#else
        handle_ = dlopen(path_str.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    }

    ~DynamicLibrary() {
        if (handle_) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
        }
    }

    DynamicLibrary(const DynamicLibrary &) = delete;
    DynamicLibrary &operator=(const DynamicLibrary &) = delete;

    DynamicLibrary(DynamicLibrary &&other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

    DynamicLibrary &operator=(DynamicLibrary &&other) noexcept {
        if (this != &other) {
            if (handle_) {
#if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(handle_));
#else
                dlclose(handle_);
#endif
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    [[nodiscard]] bool is_valid() const noexcept { return handle_ != nullptr; }

    [[nodiscard]] void *release() noexcept { return std::exchange(handle_, nullptr); }

    template <typename T>
    [[nodiscard]] T get_symbol(const std::string_view name) const {
#if defined(_WIN32)
        return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(handle_), name.data()));
#else
        return reinterpret_cast<T>(dlsym(handle_, name.data()));
#endif
    }
};

class Loader {
  public:
    [[nodiscard]]
    static std::expected<PluginHandle, LoadError> load(const std::filesystem::path &path) {
        DynamicLibrary lib{path};

        if (!lib.is_valid()) {
            return std::unexpected(
                LoadError{LoadErrorKind::NotFound, std::format("Failed to load library from OS: {}", path.string())});
        }

        auto create_fn = lib.get_symbol<CreatePluginFn>("create_plugin");
        auto destroy_fn = lib.get_symbol<DestroyPluginFn>("destroy_plugin");

        if (!create_fn || !destroy_fn) {
            return std::unexpected(LoadError{
                LoadErrorKind::MissingSymbol,
                std::format("Missing required 'create_plugin' or 'destroy_plugin' ABI symbols in: {}", path.string())});
        }

        void *self = nullptr;
        PluginVTable *vt = create_fn(&self);

        if (!vt) {
            return std::unexpected(LoadError{LoadErrorKind::InitFailed, "create_plugin returned a null vtable"});
        }

        // Transfer OS handle ownership to the PluginHandle's Deleter struct
        return PluginHandle{vt, self, destroy_fn, lib.release()};
    }
};

// Expose free function to match the invocation `plugin::load(path)` in the registry
[[nodiscard]]
inline std::expected<PluginHandle, LoadError> load(const std::filesystem::path &path) {
    return Loader::load(path);
}

} // namespace core::plugin

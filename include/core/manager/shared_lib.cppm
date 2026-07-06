module;

#include <congelado/abi.h>
#include <cstdio>
#include <dlfcn.h>

export module core_plugin:shared_lib;

import std;
import :types;
import :ffi;

export namespace core::plugin {

using types::InitFn;
using types::PluginArrayFn;
using types::PluginCountFn;
using types::PluginReadyFn;
using types::PluginStringFn;
using types::PluginUint32Fn;
using types::PluginUnloadFn;
using types::WorkerExecuteFn;
using types::WorkerTypeFn;

// ── PluginRef (plain data view) ───────────────────────────────────────────

struct PluginRef {
    // ── Symbol descriptor types ─────────────────────────────────────────

    enum class SymbolKind : std::uint8_t {
        FUNCTION,  // single function pointer — void*(*)()
        STRING_FN, // const char*(*)() — string getter
        UINT32,    // uint32_t(*)()
        SIZE_T,    // size_t(*)()
        ARRAY,     // data pointer + count pair (auto-loads "{name}" + "{name}_count")
    };

    struct SymbolInfo {
        std::string_view name;
        SymbolKind kind;
    };

    // ── Symbol lists ─────────────────────────────────────────────────────

    static constexpr SymbolInfo shared_symbols[] = {
        {.name = "congelado_init", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_type", .kind = SymbolKind::STRING_FN},
    };

    static constexpr SymbolInfo plugin_symbols[] = {
        {.name = "congelado_plugin_on_unload", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_plugin_on_ready", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_plugin_name", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_plugin_version", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_plugin_author", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_plugin_description", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_capabilities", .kind = SymbolKind::UINT32},
        {.name = "congelado_unique_type", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_requires", .kind = SymbolKind::ARRAY},
        {.name = "congelado_load_before_types", .kind = SymbolKind::ARRAY},
        {.name = "congelado_logger_write", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_logger_write_error", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_protocol_get", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_storage_get", .kind = SymbolKind::FUNCTION},
    };

    static constexpr SymbolInfo worker_symbols[] = {
        {.name = "congelado_plugin_on_unload", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_plugin_on_ready", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_plugin_name", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_plugin_version", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_capabilities", .kind = SymbolKind::UINT32},
        {.name = "congelado_requires", .kind = SymbolKind::ARRAY},
        {.name = "congelado_worker_type", .kind = SymbolKind::STRING_FN},
        {.name = "congelado_worker_execute", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_logger_write", .kind = SymbolKind::FUNCTION},
        {.name = "congelado_logger_write_error", .kind = SymbolKind::FUNCTION},
    };

    // ── Data members ────────────────────────────────────────────────────

    struct DlDeleter {
        void operator()(void *h) const {
            if (h)
                ::dlclose(h);
        }
    };
    std::unique_ptr<void, DlDeleter> m_handle;
    std::unordered_map<std::string, std::any> m_data;
};


[[nodiscard]] bool is_shared_lib(const std::filesystem::path &p);

class SharedLibrary {
  public:
    SharedLibrary() = default;

    SharedLibrary(const SharedLibrary &) = delete;
    SharedLibrary &operator=(const SharedLibrary &) = delete;

    SharedLibrary(SharedLibrary &&other) noexcept
        : m_scanned_called{other.m_scanned_called}, m_scanned{std::move(other.m_scanned)},
          m_plugins{std::move(other.m_plugins)}, m_order{std::move(other.m_order)} {
        other.m_scanned_called = false;
        other.m_scanned.clear();
        other.m_plugins.clear();
        other.m_order.clear();
    }

    SharedLibrary &operator=(SharedLibrary &&other) noexcept {
        if (this != &other) {
            close_all();
            m_scanned_called = other.m_scanned_called;
            m_scanned = std::move(other.m_scanned);
            m_plugins = std::move(other.m_plugins);
            m_order = std::move(other.m_order);
            other.m_scanned_called = false;
            other.m_scanned.clear();
            other.m_plugins.clear();
            other.m_order.clear();
        }
        return *this;
    }

    ~SharedLibrary() { close_all(); }

    // ── Phase 1: discover shared libs in directory ──────────────────────

    void scan(const std::filesystem::path &dir) {
        m_scanned_called = true;
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
            return;
        for (auto const &entry : std::filesystem::directory_iterator{dir}) {
            if (!entry.is_regular_file())
                continue;
            if (!is_shared_lib(entry.path()))
                continue;
            auto stem = entry.path().stem().string();
            if (stem.starts_with("lib"))
                stem = stem.substr(3);
            m_scanned[stem] = entry.path();
        }
    }

    // ── Phase 2: open single plugin + resolve deps recursively ─────────

    [[nodiscard]] std::expected<void, types::PluginError> open(const std::filesystem::path &path) {
        if (!m_scanned_called)
            return std::unexpected{types::PluginError::not_found("scan() not called")};

        auto stem = path.stem().string();
        if (stem.starts_with("lib"))
            stem = stem.substr(3);

        if (m_plugins.contains(stem))
            return {};

        void *raw = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!raw)
            return std::unexpected{
                types::PluginError::dlopen_failed(std::format("dlopen: {}", ::dlerror()))};

        std::unique_ptr<void, PluginRef::DlDeleter> handle(raw);
        auto ref = load_pluginref(std::move(handle), path.string());
        if (!ref->m_data.contains("congelado_init"))
            return std::unexpected{
                types::PluginError::dlopen_failed("missing congelado_init symbol")};

        auto &name = std::any_cast<const std::string &>(ref->m_data.at("name"));

        if (auto it = ref->m_data.find("congelado_requires"); it != ref->m_data.end()) {
            auto &requires_list = std::any_cast<const std::vector<std::string> &>(it->second);
            for (auto &dep_name : requires_list) {
                if (m_plugins.contains(dep_name))
                    continue;
                auto dep_it = m_scanned.find(dep_name);
                if (dep_it == m_scanned.end())
                    return std::unexpected{types::PluginError::not_found(
                        std::format("dependency '{}' not found for '{}'", dep_name, name))};
                auto res = open(dep_it->second);
                if (!res)
                    return std::unexpected{types::PluginError::not_found(
                        std::format("dependency '{}' failed to load for '{}': {}",
                                    dep_name, name, res.error().get_message()))};
            }
        }

        m_plugins.emplace(name, ref);
        m_order.push_back(name);
        return {};
    }

    // ── Phase 2: open all scanned plugins ──────────────────────────────

    [[nodiscard]] std::expected<void, types::PluginError> open_all() {
        for (auto &[name, path] : m_scanned) {
            auto res = open(path);
            if (!res)
                return std::unexpected{std::move(res.error())};
        }
        return {};
    }

    // ── Phase 3: init all opened plugins ───────────────────────────────

    [[nodiscard]] std::expected<std::vector<FfiRuntime>, types::PluginError>
    build(const CongeladoHostCallbacks &host_cb,
          const std::unordered_map<std::string, types::GenerationConfig> &configs) {
        if (!m_scanned_called)
            return std::unexpected{types::PluginError::not_found("scan() not called")};

        std::vector<FfiRuntime> runtimes;

        for (auto &name : m_order) {
            auto ref = find(name);
            if (!ref)
                continue;

            types::GenerationConfig default_cfg;
            const types::GenerationConfig *gen_cfg = &default_cfg;
            if (auto cfg_it = configs.find(name); cfg_it != configs.end())
                gen_cfg = &cfg_it->second;

            types::ConfigViewBuilder cfg_view;
            cfg_view.add("runtimes",
                         std::to_string(std::to_underlying(gen_cfg->get_runtimes())));
            cfg_view.add("python_module",
                         std::string{gen_cfg->get_python_config().get_module_name()});
            cfg_view.add("lua_table",
                         std::string{gen_cfg->get_lua_config().get_table_name()});
            cfg_view.add("lua_safe_mode",
                         gen_cfg->get_lua_config().get_safe_mode() ? "true" : "false");
            for (auto &[k, v] : gen_cfg->get_extra())
                cfg_view.add(k, v);

            auto view = cfg_view.view();

            if (reinterpret_cast<InitFn>(std::any_cast<void *>(ref->m_data.at("congelado_init")))(
                    &host_cb, &view) != 0)
                return std::unexpected{types::PluginError::dlopen_failed(
                    std::format("congelado_init failed for '{}'", name))};

            runtimes.emplace_back(*gen_cfg);
        }

        for (auto &name : m_order)
            if (auto ref = find(name); ref)
                if (auto it = ref->m_data.find("congelado_plugin_on_ready");
                    it != ref->m_data.end())
                    reinterpret_cast<PluginReadyFn>(std::any_cast<void *>(it->second))();

        return runtimes;
    }

    // ── Accessors ───────────────────────────────────────────────────────

    [[nodiscard]] std::shared_ptr<PluginRef> find(std::string_view name) noexcept {
        auto it = m_plugins.find(std::string{name});
        return it != m_plugins.end() ? it->second : nullptr;
    }

    [[nodiscard]] std::shared_ptr<const PluginRef> find(std::string_view name) const noexcept {
        auto it = m_plugins.find(std::string{name});
        return it != m_plugins.end() ? it->second : nullptr;
    }

    [[nodiscard]] std::size_t get_count() const noexcept { return m_plugins.size(); }

    void close_all() noexcept {
        for (auto it = m_order.rbegin(); it != m_order.rend(); ++it) {
            if (auto pit = m_plugins.find(*it); pit != m_plugins.end()) {
                if (auto it2 = pit->second->m_data.find("congelado_plugin_on_unload");
                    it2 != pit->second->m_data.end())
                    reinterpret_cast<PluginUnloadFn>(std::any_cast<void *>(it2->second))();
            }
        }
        m_plugins.clear();
        m_order.clear();
    }

    // ── Iteration (insertion order via m_order) ─────────────────────────

    template <typename Self, std::invocable<PluginRef &> Fn>
    void for_each(this Self &&self, Fn &&fn) {
        for (auto &name : self.m_order) {
            if (auto it = self.m_plugins.find(name); it != self.m_plugins.end())
                std::invoke(std::forward<Fn>(fn), *it->second);
        }
    }

  private:
    // ── Private: typed dlsym wrapper ────────────────────────────────────

    template <typename F>
    [[nodiscard]] F sym(void *handle, std::string_view name) const noexcept {
        return reinterpret_cast<F>(::dlsym(handle, name.data()));
    }


    void load_symbols(void *handle, std::span<const PluginRef::SymbolInfo> symbols,
                      std::shared_ptr<PluginRef> &ref) {
        for (auto &info : symbols) {
            auto *ptr = sym<void *>(handle, info.name);
            if (!ptr)
                continue;

            switch (info.kind) {

            case PluginRef::SymbolKind::FUNCTION:
                ref->m_data[std::string{info.name}] = ptr;
                break;

            case PluginRef::SymbolKind::STRING_FN: {
                auto val = sym<PluginStringFn>(handle, info.name)();
                ref->m_data[std::string{info.name}] = std::string{val ? val : ""};
                break;
            }

            case PluginRef::SymbolKind::UINT32:
                ref->m_data[std::string{info.name}] = sym<PluginUint32Fn>(handle, info.name)();
                break;

            case PluginRef::SymbolKind::ARRAY: {
                auto *data_fn = sym<PluginArrayFn>(handle, info.name);
                std::string count_name = std::string{info.name} + "_count";
                auto *count_fn = sym<PluginCountFn>(handle, count_name);
                if (!data_fn || !count_fn)
                    break;
                auto count = count_fn();
                auto *items = data_fn();
                std::vector<std::string> vec;
                vec.reserve(count);
                for (std::size_t j = 0; j < count; ++j)
                    vec.emplace_back(items[j]);
                ref->m_data[std::string{info.name}] = std::move(vec);
                break;
            }

            case PluginRef::SymbolKind::SIZE_T:
                break;
            }
        }
    }

    // ── Private: load plugin from handle ────────────────────────────────

    [[nodiscard]] std::shared_ptr<PluginRef>
    load_pluginref(std::unique_ptr<void, PluginRef::DlDeleter> handle, std::string file_path) {
        auto ref = std::make_shared<PluginRef>();
        auto *raw_handle = handle.get();
        ref->m_handle = std::move(handle);
        ref->m_data["path"] = std::move(file_path);
        auto stem = std::filesystem::path{std::any_cast<const std::string &>(ref->m_data["path"])}
                        .stem()
                        .string();
        if (stem.starts_with("lib"))
            stem = stem.substr(3);

        load_symbols(raw_handle, PluginRef::shared_symbols, ref);

        auto type_it = ref->m_data.find("congelado_type");
        auto &type = (type_it != ref->m_data.end())
                         ? std::any_cast<const std::string &>(type_it->second)
                         : s_empty_type;
        if (type == "worker")
            load_symbols(raw_handle, PluginRef::worker_symbols, ref);
        else
            load_symbols(raw_handle, PluginRef::plugin_symbols, ref);

        if (ref->m_data.find("name") == ref->m_data.end())
            ref->m_data["name"] = std::move(stem);
        return ref;
    }

    static inline const std::string s_empty_type;

    bool m_scanned_called{false};
    std::unordered_map<std::string, std::filesystem::path> m_scanned;
    std::unordered_map<std::string, std::shared_ptr<PluginRef>> m_plugins;
    std::vector<std::string> m_order;
};

} // namespace core::plugin

export namespace core::plugin {

[[nodiscard]] inline bool is_shared_lib(const std::filesystem::path &p) {
    auto ext = p.extension().string();
#if defined(_WIN32)
    return ext == ".dll";
#elif defined(__APPLE__)
    return ext == ".dylib";
#else
    return ext == ".so";
#endif
}

} // namespace core::plugin

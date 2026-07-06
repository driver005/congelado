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
using FfiRuntime;
using types::PluginRef;

class SharedLibrary {
  public:
    SharedLibrary() = default;

    SharedLibrary(const SharedLibrary &) = delete;
    SharedLibrary &operator=(const SharedLibrary &) = delete;

    SharedLibrary(SharedLibrary &&other) noexcept
        : m_scanned_called{other.m_scanned_called}, m_scanned{std::move(other.m_scanned)},
          m_runtimes{std::move(other.m_runtimes)}, m_order{std::move(other.m_order)} {
        other.m_scanned_called = false;
        other.m_scanned.clear();
        other.m_runtimes.clear();
        other.m_order.clear();
    }

    SharedLibrary &operator=(SharedLibrary &&other) noexcept {
        if (this != &other) {
            close_all();
            m_scanned_called = other.m_scanned_called;
            m_scanned = std::move(other.m_scanned);
            m_runtimes = std::move(other.m_runtimes);
            m_order = std::move(other.m_order);
            other.m_scanned_called = false;
            other.m_scanned.clear();
            other.m_runtimes.clear();
            other.m_order.clear();
        }
        return *this;
    }

    ~SharedLibrary() { close_all(); }

    static bool is_shared_lib(const std::filesystem::path &p) {
        auto ext = p.extension().string();
    #if defined(_WIN32)
        return ext == ".dll";
    #elif defined(__APPLE__)
        return ext == ".dylib";
    #else
        return ext == ".so";
    #endif
    }

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

    void open_or_throw(const std::filesystem::path &path) {
        if (!m_scanned_called)
            throw types::PluginError::not_found("scan() not called");

        auto stem = path.stem().string();
        if (stem.starts_with("lib"))
            stem = stem.substr(3);

        if (m_runtimes.contains(stem))
            return;

        void *raw = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!raw)
            throw types::PluginError::dlopen_failed(std::format("dlopen: {}", ::dlerror()));

        std::unique_ptr<void, types::PluginRef::DlDeleter> handle(raw);
        auto pref = load_pluginref(std::move(handle), path.string());
        if (!pref->m_data.contains(std::string{types::PluginRef::shared_symbol_name(0)}))
            throw types::PluginError::dlopen_failed("missing congelado_init symbol");

        auto &name = std::any_cast<const std::string &>(pref->m_data.at("name"));

        if (auto it = pref->m_data.find("congelado_requires"); it != pref->m_data.end()) {
            auto &requires_list = std::any_cast<const std::vector<std::string> &>(it->second);
            for (auto &dep_name : requires_list) {
                if (m_runtimes.contains(dep_name))
                    continue;
                auto dep_it = m_scanned.find(dep_name);
                if (dep_it == m_scanned.end())
                    throw types::PluginError::not_found(std::format("dependency '{}' not found for '{}'", dep_name, name));
                open_or_throw(dep_it->second);
            }
        }

        auto runtime = std::make_shared<FfiRuntime>();
        runtime->attach_plugin(pref);
        m_runtimes.emplace(name, runtime);
        m_order.push_back(name);
    }

    [[nodiscard]] void build(const CongeladoHostCallbacks &host_cb,
          const std::unordered_map<std::string, types::GenerationConfig> &configs) {
        if (!m_scanned_called)
            throw types::PluginError::not_found("scan() not called");

        for (auto &name : m_order) {
            auto it = m_runtimes.find(name);
            if (it == m_runtimes.end())
                continue;
            auto runtime = it->second;

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

            if (runtime->init(&host_cb, &view) != 0)
                throw types::PluginError::dlopen_failed(std::format("congelado_init failed for '{}'", name));
        }

        for (auto &name : m_order) {
            if (auto it = m_runtimes.find(name); it != m_runtimes.end())
                it->second->on_ready();
        }
    }

    void close_all() noexcept {
        for (auto it = m_order.rbegin(); it != m_order.rend(); ++it) {
            if (auto rit = m_runtimes.find(*it); rit != m_runtimes.end()) {
                rit->second->on_unload();
            }
        }
        m_runtimes.clear();
        m_order.clear();
    }

    [[nodiscard]] std::shared_ptr<FfiRuntime> find(std::string_view name) noexcept {
        auto it = m_runtimes.find(std::string{name});
        return it != m_runtimes.end() ? it->second : nullptr;
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
        auto stem = std::filesystem::path{std::any_cast<const std::string &>(ref->m_data["path"]) }
                        .stem()
                        .string();
        if (stem.starts_with("lib"))
            stem = stem.substr(3);

        load_symbols(raw_handle, PluginRef::shared_symbols, ref);

        auto type_it = ref->m_data.find(std::string{PluginRef::shared_symbol_name(1)});
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
    std::unordered_map<std::string, std::shared_ptr<FfiRuntime>> m_runtimes;
    std::vector<std::string> m_order;
};

} // namespace core::plugin

module;

#include <cstdio>
#include <ffi.h>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#define CONGELADO_HOST
#include <congelado/plugin.h>

export module core_plugin:loader;

import std;
import interfaces;
import congelado_plugin;
import core_logger;
import core_config;

// ── Module-private implementation detail ─────────────────────────────────────
// BridgeImpl and ManagedPlugin are not exported. PluginManager uses them
// internally; external code only sees PluginManager and Cap.

namespace core::plugin::detail {

// RAII wrapper around one libffi closure.
class Closure {
  public:
    using Thunk = void (*)(ffi_cif *, void *, void **, void *);

    Closure(std::initializer_list<ffi_type *> arg_types, Thunk thunk, void *user_data)
        : m_arg_types{arg_types} {
        if (ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, static_cast<unsigned>(m_arg_types.size()),
                         &ffi_type_void, m_arg_types.data()) != FFI_OK)
            throw std::runtime_error{"ffi_prep_cif failed"};
        m_raw = static_cast<ffi_closure *>(ffi_closure_alloc(sizeof(ffi_closure), &m_code));
        if (m_raw == nullptr)
            throw std::bad_alloc{};
        if (ffi_prep_closure_loc(m_raw, &m_cif, thunk, user_data, m_code) != FFI_OK)
            throw std::runtime_error{"ffi_prep_closure_loc failed"};
    }

    ~Closure() {
        if (m_raw != nullptr)
            ffi_closure_free(m_raw);
    }

    Closure(const Closure &) = delete;
    Closure &operator=(const Closure &) = delete;
    Closure(Closure &&) = delete;
    Closure &operator=(Closure &&) = delete;

    [[nodiscard]] void *get() const noexcept { return m_code; }

  private:
    std::vector<ffi_type *> m_arg_types;
    ffi_cif m_cif{};
    ffi_closure *m_raw{nullptr};
    void *m_code{nullptr};
};

struct PluginSymbols {
    using NameFn                = const char *(*)() noexcept;
    using VersionFn             = const char *(*)() noexcept;
    using CapsFn                = uint32_t (*)() noexcept;
    using OnLoadFn              = void (*)(const CongeladoHostCallbacks *, const CongeladoConfigView *);
    using OnUnloadFn            = void (*)() noexcept;
    using LogWriteFn            = void (*)(int, const char *, std::size_t) noexcept;
    using LogWriteErrFn         = void (*)(const char *, std::size_t) noexcept;
    using ProtoGetFn            = void *(*)() noexcept;
    using StorageGetFn          = void *(*)() noexcept;
    using UniqueTypeFn          = const char *(*)() noexcept;
    using RequiresFn            = const char *const *(*)() noexcept;
    using RequiresCountFn       = std::size_t (*)() noexcept;
    using LoadBeforeTypesFn     = const char *const *(*)() noexcept;
    using LoadBeforeTypesCountFn = std::size_t (*)() noexcept;

    NameFn                name{nullptr};
    VersionFn             version{nullptr};
    CapsFn                capabilities{nullptr};
    OnLoadFn              on_load{nullptr};
    OnUnloadFn            on_unload{nullptr};
    LogWriteFn            logger_write{nullptr};
    LogWriteErrFn         logger_write_error{nullptr};
    ProtoGetFn            protocol_get{nullptr};
    StorageGetFn          storage_get{nullptr};
    UniqueTypeFn          unique_type{nullptr};
    RequiresFn            requires_get{nullptr};
    RequiresCountFn       requires_count{nullptr};
    LoadBeforeTypesFn     load_before_types_get{nullptr};
    LoadBeforeTypesCountFn load_before_types_count{nullptr};
};

// Loads one plugin .so via dlopen + libffi. Implements ILogger for logger plugins.
// Two-phase: open() probes symbols; activate() builds closures + calls congelado_on_load.
class BridgeImpl : public interfaces::ILogger {
  public:
    BridgeImpl(const BridgeImpl &) = delete;
    BridgeImpl &operator=(const BridgeImpl &) = delete;
    BridgeImpl(BridgeImpl &&) = delete;
    BridgeImpl &operator=(BridgeImpl &&) = delete;

    [[nodiscard]] static std::expected<std::shared_ptr<BridgeImpl>, std::string>
    open(const std::filesystem::path &path, const core::config::PluginConfig *plugin_cfg = nullptr) {
        void *lib = open_lib(path);
        if (lib == nullptr)
            return std::unexpected{std::format("dlopen failed: {}", path.string())};

        auto bridge = std::shared_ptr<BridgeImpl>(new BridgeImpl{lib});

        if (auto err = bridge->resolve_symbols(); !err.empty())
            return std::unexpected{std::move(err)};

        bridge->m_lib_name    = bridge->m_syms.name();
        bridge->m_lib_version = bridge->m_syms.version();
        bridge->build_config_view(plugin_cfg);
        bridge->read_metadata();
        return bridge;
    }

    void activate(void *router_ctx = nullptr, void *controller_ctx = nullptr,
                  void *leverager_ctx = nullptr) {
        try {
            m_log_closure = std::make_unique<Closure>(
                std::initializer_list<ffi_type *>{&ffi_type_pointer, &ffi_type_sint,
                                                  &ffi_type_pointer, size_ffi_type()},
                &BridgeImpl::log_thunk, this);
            m_sched_closure = std::make_unique<Closure>(
                std::initializer_list<ffi_type *>{&ffi_type_pointer},
                &BridgeImpl::schedule_thunk, this);
        } catch (const std::exception &ex) {
            std::println(stderr, "[loader::{}] closure setup failed: {}", m_lib_name, ex.what());
            return;
        }

        CongeladoHostCallbacks callbacks{
            .log          = reinterpret_cast<congelado_log_fn>(m_log_closure->get()),
            .schedule     = reinterpret_cast<congelado_sched_fn>(m_sched_closure->get()),
            .router_ctx   = router_ctx,
            .controller_ctx = controller_ctx,
            .leverager_ctx  = leverager_ctx,
            .ctx            = this,
        };

        if (m_syms.on_load != nullptr)
            m_syms.on_load(&callbacks, &m_cfg_view);

        m_caps = (m_syms.capabilities != nullptr) ? m_syms.capabilities() : 0;
    }

    ~BridgeImpl() override {
        release();
        close_lib();
    }

    void release() noexcept {
        if (m_syms.on_unload != nullptr) {
            m_syms.on_unload();
            m_syms = {};
        }
    }

    [[nodiscard]] std::string_view get_name()        const noexcept { return m_lib_name; }
    [[nodiscard]] std::string_view get_version()     const noexcept { return m_lib_version; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept { return m_unique_type; }
    [[nodiscard]] std::uint32_t    capabilities()    const noexcept { return m_caps; }
    [[nodiscard]] std::span<const std::string> get_requires()         const noexcept { return m_requires; }
    [[nodiscard]] std::span<const std::string> get_load_before_types() const noexcept { return m_load_before_types; }

    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        if (m_syms.logger_write != nullptr)
            m_syms.logger_write(static_cast<int>(level), msg.data(), msg.size());
    }
    void error(std::string_view msg) noexcept override {
        if (m_syms.logger_write_error != nullptr)
            m_syms.logger_write_error(msg.data(), msg.size());
    }

  private:
    explicit BridgeImpl(void *lib) : m_lib{lib} {}

    static void log_thunk(ffi_cif * /*cif*/, void * /*ret*/, void **args,
                          void *user_data) noexcept {
        auto *self    = static_cast<BridgeImpl *>(user_data);
        auto  level   = *static_cast<int *>(args[1]);
        auto *msg_ptr = *static_cast<const char **>(args[2]);
        auto  msg_len = *static_cast<std::size_t *>(args[3]);
        std::println(stderr, "[plugin::{}] log({}): {}", self->m_lib_name, level,
                     std::string_view{msg_ptr, msg_len});
    }

    static void schedule_thunk(ffi_cif * /*cif*/, void * /*ret*/, void ** /*args*/,
                               void *user_data) noexcept {
        std::println(stderr, "[plugin::{}] schedule requested",
                     static_cast<BridgeImpl *>(user_data)->m_lib_name);
    }

    [[nodiscard]] static void *open_lib(const std::filesystem::path &path) noexcept {
#if defined(_WIN32)
        return static_cast<void *>(LoadLibraryA(path.string().c_str()));
#else
        return dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    }

    void close_lib() noexcept {
        if (m_lib == nullptr) return;
#if defined(_WIN32)
        FreeLibrary(static_cast<HMODULE>(m_lib));
#else
        dlclose(m_lib);
#endif
        m_lib = nullptr;
    }

    template <typename Fn>
    [[nodiscard]] Fn probe(const char *sym) const noexcept {
#if defined(_WIN32)
        return reinterpret_cast<Fn>(GetProcAddress(static_cast<HMODULE>(m_lib), sym));
#else
        return reinterpret_cast<Fn>(dlsym(m_lib, sym));
#endif
    }

    [[nodiscard]] static ffi_type *size_ffi_type() noexcept {
        return sizeof(std::size_t) == 8 ? &ffi_type_uint64 : &ffi_type_uint32;
    }

    [[nodiscard]] std::string resolve_symbols() noexcept {
        m_syms.name         = probe<PluginSymbols::NameFn>("congelado_plugin_name");
        m_syms.version      = probe<PluginSymbols::VersionFn>("congelado_plugin_version");
        m_syms.capabilities = probe<PluginSymbols::CapsFn>("congelado_capabilities");

        if (m_syms.name == nullptr || m_syms.version == nullptr || m_syms.capabilities == nullptr)
            return "missing required symbols: congelado_plugin_name / congelado_plugin_version / congelado_capabilities";

        m_syms.on_load              = probe<PluginSymbols::OnLoadFn>("congelado_on_load");
        m_syms.on_unload            = probe<PluginSymbols::OnUnloadFn>("congelado_on_unload");
        m_syms.logger_write         = probe<PluginSymbols::LogWriteFn>("congelado_logger_write");
        m_syms.logger_write_error   = probe<PluginSymbols::LogWriteErrFn>("congelado_logger_write_error");
        m_syms.protocol_get         = probe<PluginSymbols::ProtoGetFn>("congelado_protocol_get");
        m_syms.storage_get          = probe<PluginSymbols::StorageGetFn>("congelado_storage_get");
        m_syms.unique_type          = probe<PluginSymbols::UniqueTypeFn>("congelado_unique_type");
        m_syms.requires_get         = probe<PluginSymbols::RequiresFn>("congelado_requires");
        m_syms.requires_count       = probe<PluginSymbols::RequiresCountFn>("congelado_requires_count");
        m_syms.load_before_types_get   = probe<PluginSymbols::LoadBeforeTypesFn>("congelado_load_before_types");
        m_syms.load_before_types_count = probe<PluginSymbols::LoadBeforeTypesCountFn>("congelado_load_before_types_count");
        return {};
    }

    void build_config_view(const core::config::PluginConfig *plugin_cfg) noexcept {
        if (plugin_cfg == nullptr) return;
        m_cfg_keys.clear();
        m_cfg_vals.clear();
        for (const auto &[key, value] : plugin_cfg->get_fields()) {
            m_cfg_keys.push_back(key.c_str());
            m_cfg_vals.push_back(value.c_str());
        }
        m_cfg_view = CongeladoConfigView{
            .keys   = m_cfg_keys.data(),
            .values = m_cfg_vals.data(),
            .count  = m_cfg_keys.size(),
        };
    }

    void read_metadata() noexcept {
        if (m_syms.unique_type != nullptr) {
            const char *utype = m_syms.unique_type();
            m_unique_type = (utype != nullptr) ? std::string{utype} : "";
        }
        if (m_syms.requires_count != nullptr && m_syms.requires_get != nullptr) {
            const auto count = m_syms.requires_count();
            const auto *arr  = m_syms.requires_get();
            if (arr != nullptr) {
                m_requires.reserve(count);
                for (std::size_t i = 0; i < count; ++i)
                    if (arr[i] != nullptr) m_requires.emplace_back(arr[i]);
            }
        }
        if (m_syms.load_before_types_count != nullptr && m_syms.load_before_types_get != nullptr) {
            const auto count = m_syms.load_before_types_count();
            const auto *arr  = m_syms.load_before_types_get();
            if (arr != nullptr) {
                m_load_before_types.reserve(count);
                for (std::size_t i = 0; i < count; ++i)
                    if (arr[i] != nullptr) m_load_before_types.emplace_back(arr[i]);
            }
        }
    }

    void *m_lib{nullptr};
    PluginSymbols m_syms{};
    std::string m_lib_name;
    std::string m_lib_version;
    std::string m_unique_type;
    std::vector<std::string> m_requires;
    std::vector<std::string> m_load_before_types;
    std::uint32_t m_caps{0};
    std::unique_ptr<Closure> m_log_closure;
    std::unique_ptr<Closure> m_sched_closure;
    std::vector<const char *> m_cfg_keys;
    std::vector<const char *> m_cfg_vals;
    CongeladoConfigView m_cfg_view{};
};

// Adapter: wraps BridgeImpl and exposes it as congelado::Plugin.
// Constructed by PluginManager::addPlugin(); not directly accessible outside loader.cppm.
class ManagedPlugin final : public congelado::Plugin {
  public:
    explicit ManagedPlugin(std::shared_ptr<BridgeImpl> bridge)
        : m_bridge{std::move(bridge)} {
        for (const auto &req : m_bridge->get_requires())
            m_requires_views.emplace_back(req);
        for (const auto &tag : m_bridge->get_load_before_types())
            m_load_before_views.emplace_back(tag);
    }

    [[nodiscard]] std::string_view get_name()    const noexcept override { return m_bridge->get_name(); }
    [[nodiscard]] std::string_view get_version() const noexcept override { return m_bridge->get_version(); }
    [[nodiscard]] std::uint32_t    capabilities() const noexcept override { return m_bridge->capabilities(); }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return m_bridge->get_unique_type(); }

    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        return m_requires_views;
    }
    [[nodiscard]] std::span<const std::string_view> get_load_before_types() const noexcept override {
        return m_load_before_views;
    }

    void on_load(congelado::HostCallbacks const & /*host*/,
                 congelado::ConfigView const & /*cfg*/) override {}
    void on_unload() override { m_bridge->release(); }

    [[nodiscard]] BridgeImpl &bridge() noexcept { return *m_bridge; }
    [[nodiscard]] std::shared_ptr<BridgeImpl> bridge_ptr() noexcept { return m_bridge; }

  private:
    std::shared_ptr<BridgeImpl> m_bridge;
    std::vector<std::string_view> m_requires_views;
    std::vector<std::string_view> m_load_before_views;
};

} // namespace core::plugin::detail

// ── Public exports ────────────────────────────────────────────────────────────

export namespace core::plugin {

enum class Cap : std::uint32_t {
    LOGGER   = CONGELADO_CAP_LOGGER,
    PROTOCOL = CONGELADO_CAP_PROTOCOL,
    STORAGE  = CONGELADO_CAP_STORAGE,
    CUSTOM   = CONGELADO_CAP_CUSTOM,
};

// Owns all loaded plugins. Only component allowed to use BridgeImpl/ManagedPlugin.
class PluginManager {
  public:
    PluginManager() = default;
    ~PluginManager() {
        for (auto it = m_activation_order.rbegin(); it != m_activation_order.rend(); ++it)
            (*it)->on_unload();
    }
    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;
    PluginManager(PluginManager &&) = default;
    PluginManager &operator=(PluginManager &&) = default;

    // Phase 1: open one .so, probe symbols, read metadata. Does not call on_load.
    void addPlugin(const std::filesystem::path &path,
                   const core::config::PluginConfig *config = nullptr) {
        auto result = detail::BridgeImpl::open(path, config);
        if (!result) {
            std::println(stderr, "[manager] failed to open '{}': {}", path.string(),
                         result.error());
            return;
        }
        m_plugins.push_back(
            std::make_unique<detail::ManagedPlugin>(std::move(*result)));
    }

    // Phases 2–4: uniqueness filter → Kahn sort → activate in order.
    // Registers logger plugins with LoggerRegistry.
    void activate(void *router_ctx, void *controller_ctx, void *leverager_ctx) {
        // Phase 2: uniqueness filter
        std::unordered_map<std::string, std::string> seen_types;
        std::vector<detail::ManagedPlugin *> surviving;
        for (auto &plugin : m_plugins) {
            std::string utype{plugin->bridge().get_unique_type()};
            if (utype.empty()) {
                surviving.push_back(plugin.get());
                continue;
            }
            if (!seen_types.contains(utype)) {
                seen_types[utype] = std::string{plugin->bridge().get_name()};
                surviving.push_back(plugin.get());
            } else {
                std::println(stderr,
                             "[manager] '{}' skipped — unique type '{}' already claimed by '{}'",
                             plugin->bridge().get_name(), utype, seen_types[utype]);
            }
        }

        // Phase 3: Kahn's topological sort
        std::unordered_map<std::string, detail::ManagedPlugin *> name_map;
        for (auto *plugin : surviving)
            name_map[std::string{plugin->bridge().get_name()}] = plugin;

        for (auto *plugin : surviving) {
            for (const auto &req : plugin->bridge().get_requires()) {
                if (!name_map.contains(req)) {
                    std::println(stderr, "[manager] '{}' requires '{}' which is not loaded",
                                 plugin->bridge().get_name(), req);
                    std::abort();
                }
            }
        }

        std::unordered_map<std::string, int> in_degree;
        std::unordered_map<std::string, std::vector<std::string>> dependents;
        for (auto *plugin : surviving) {
            std::string name{plugin->bridge().get_name()};
            in_degree.try_emplace(name, 0);
            for (const auto &req : plugin->bridge().get_requires()) {
                dependents[req].push_back(name);
                ++in_degree[name];
            }
        }

        {
            std::unordered_map<std::string, std::vector<std::string>> type_to_names;
            for (auto *plugin : surviving) {
                std::string utype{plugin->bridge().get_unique_type()};
                if (!utype.empty())
                    type_to_names[utype].push_back(std::string{plugin->bridge().get_name()});
            }
            for (auto *plugin : surviving) {
                std::string from{plugin->bridge().get_name()};
                for (const auto &type_tag : plugin->bridge().get_load_before_types()) {
                    auto it = type_to_names.find(std::string{type_tag});
                    if (it == type_to_names.end()) continue;
                    for (const auto &target : it->second) {
                        if (target == from) continue;
                        auto &deps = dependents[from];
                        if (std::ranges::find(deps, target) == deps.end()) {
                            deps.push_back(target);
                            ++in_degree[target];
                        }
                    }
                }
            }
        }

        std::queue<std::string> ready;
        for (auto &[name, deg] : in_degree)
            if (deg == 0) ready.push(name);

        std::vector<detail::ManagedPlugin *> sorted;
        sorted.reserve(surviving.size());
        while (!ready.empty()) {
            auto name = ready.front();
            ready.pop();
            sorted.push_back(name_map[name]);
            for (auto &dep : dependents[name])
                if (--in_degree[dep] == 0) ready.push(dep);
        }

        if (sorted.size() != surviving.size()) {
            std::println(stderr, "[manager] plugin dependency cycle — aborting");
            std::abort();
        }

        // Phase 4: activate in sorted order
        for (auto *plugin : sorted) {
            plugin->bridge().activate(router_ctx, controller_ctx, leverager_ctx);
            m_activation_order.push_back(plugin);
            m_plugin_ptrs.push_back(plugin);
            std::println("[manager] loaded plugin '{}'", plugin->bridge().get_name());

            if ((plugin->bridge().capabilities() & CONGELADO_CAP_LOGGER) != 0) {
                core::logger::LoggerRegistry::register_logger(plugin->bridge_ptr());
            }
        }
    }

    [[nodiscard]] congelado::Plugin *getByCapability(Cap cap) const noexcept {
        for (auto *ptr : m_plugin_ptrs) {
            if ((ptr->capabilities() & std::to_underlying(cap)) != 0) return ptr;
        }
        return nullptr;
    }

    [[nodiscard]] std::span<congelado::Plugin *const> getAll() const noexcept {
        return m_plugin_ptrs;
    }

  private:
    std::vector<std::unique_ptr<detail::ManagedPlugin>> m_plugins;
    std::vector<detail::ManagedPlugin *> m_activation_order;
    std::vector<congelado::Plugin *> m_plugin_ptrs;
};

} // namespace core::plugin

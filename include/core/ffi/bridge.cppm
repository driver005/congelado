module;

#include <cstdio>
#include <ffi.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "core/ffi/plugin_api.h"

export module core_ffi:bridge;

import std;
import shared;
import interfaces;
import core_config;

// ── Module-private ────────────────────────────────────────────────────────────

namespace core::ffi {

// RAII wrapper around one libffi closure.
// Owns the ffi_closure allocation and the arg_types array the CIF points into.
class Closure {
  public:
    using Thunk = void (*)(ffi_cif *, void *, void **, void *);

    Closure(std::initializer_list<ffi_type *> arg_types, Thunk thunk, void *user_data)
        : m_arg_types{arg_types} {
        if (ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI,
                         static_cast<unsigned>(m_arg_types.size()),
                         &ffi_type_void,
                         m_arg_types.data()) != FFI_OK)
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
    std::vector<ffi_type *> m_arg_types; // must outlive m_cif
    ffi_cif m_cif{};
    ffi_closure *m_raw{nullptr};
    void *m_code{nullptr};
};

// Resolved function pointers for a loaded plugin.
// All symbols are optional except name, version, capabilities.
struct PluginSymbols {
    using NameFn        = const char *(*)() noexcept;
    using VersionFn     = const char *(*)() noexcept;
    using CapsFn        = uint32_t (*)() noexcept;
    using OnLoadFn      = void (*)(const CongeladoHostCallbacks *, const CongeladoConfigView *);
    using OnUnloadFn    = void (*)() noexcept;
    using LogWriteFn    = void (*)(int, const char *, size_t) noexcept;
    using LogWriteErrFn = void (*)(const char *, size_t) noexcept;
    using ProtoGetFn    = void *(*)() noexcept;

    NameFn        name{nullptr};
    VersionFn     version{nullptr};
    CapsFn        capabilities{nullptr};
    OnLoadFn      on_load{nullptr};
    OnUnloadFn    on_unload{nullptr};
    LogWriteFn    logger_write{nullptr};
    LogWriteErrFn logger_write_error{nullptr};
    ProtoGetFn    protocol_get{nullptr};
};

} // namespace core::ffi

// ── Exported ──────────────────────────────────────────────────────────────────

export namespace core::ffi {

// Capability bitmask — mirrors CONGELADO_CAP_* defines in plugin_api.h.
enum class Cap : std::uint32_t {
    LOGGER   = CONGELADO_CAP_LOGGER,
    PROTOCOL = CONGELADO_CAP_PROTOCOL,
    CUSTOM   = CONGELADO_CAP_CUSTOM,
};

class LoadError {
  public:
    explicit LoadError(std::string detail) : m_detail{std::move(detail)} {}

    [[nodiscard]] std::string_view get_detail() const noexcept { return m_detail; }

  private:
    std::string m_detail;
};

// RAII wrapper around one loaded plugin .so.
//
// Loading sequence:
//   1. dlopen the .so
//   2. Resolve well-known symbols via dlsym into a PluginSymbols table
//   3. Build libffi closures for host callbacks (log, schedule)
//   4. Call congelado_on_load(callbacks, cfg_view)
//   5. Read capability bitmask from congelado_capabilities()
//   6. Cache protocol pointer if CONGELADO_CAP_PROTOCOL is set
class FfiBridge : public shared::HandlerBase,
                  public interfaces::ILogger,
                  public std::enable_shared_from_this<FfiBridge> {
  public:
    FfiBridge(const FfiBridge &) = delete;
    FfiBridge &operator=(const FfiBridge &) = delete;
    FfiBridge(FfiBridge &&) = delete;
    FfiBridge &operator=(FfiBridge &&) = delete;

    [[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
    load(const std::filesystem::path &path,
         const core::config::PluginConfig *plugin_cfg = nullptr,
         void *router_ctx = nullptr) {
        void *lib = open_lib(path);
        if (lib == nullptr)
            return std::unexpected(LoadError{std::format("dlopen failed: {}", path.string())});

        auto bridge = std::shared_ptr<FfiBridge>(new FfiBridge{lib});

        if (auto err = bridge->resolve_symbols(); !err.empty())
            return std::unexpected(LoadError{std::move(err)});

        bridge->m_lib_name = bridge->m_syms.name();

        try {
            bridge->m_log_closure = std::make_unique<Closure>(
                std::initializer_list<ffi_type *>{
                    &ffi_type_pointer, &ffi_type_sint, &ffi_type_pointer, size_ffi_type()},
                &FfiBridge::log_thunk, bridge.get());
            bridge->m_sched_closure = std::make_unique<Closure>(
                std::initializer_list<ffi_type *>{&ffi_type_pointer},
                &FfiBridge::schedule_thunk, bridge.get());
        } catch (const std::exception &ex) {
            return std::unexpected(LoadError{std::format("ffi closure setup failed: {}", ex.what())});
        }

        CongeladoHostCallbacks callbacks{
            .log        = reinterpret_cast<CongeladoLogFn>(bridge->m_log_closure->get()),
            .schedule   = reinterpret_cast<CongeladoScheduleFn>(bridge->m_sched_closure->get()),
            .router_ctx = router_ctx,
            .ctx        = bridge.get(),
        };

        if (bridge->m_syms.on_load != nullptr)
            bridge->m_syms.on_load(&callbacks, bridge->build_config_view(plugin_cfg));

        bridge->discover_caps();
        return bridge;
    }

    ~FfiBridge() override {
        release_plugin();
        close_lib();
    }

    [[nodiscard]] bool has(Cap cap) const noexcept {
        return (m_caps & std::to_underlying(cap)) != 0;
    }

    [[nodiscard]] std::shared_ptr<interfaces::IProtocol> get_protocol() const noexcept {
        return m_protocol;
    }

    [[nodiscard]] std::string_view name() const noexcept override { return m_lib_name; }

    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        if (m_syms.logger_write == nullptr) return;
        m_syms.logger_write(static_cast<int>(level), msg.data(), msg.size());
    }

    void error(std::string_view msg) noexcept override {
        if (m_syms.logger_write_error == nullptr) return;
        m_syms.logger_write_error(msg.data(), msg.size());
    }

    [[nodiscard]] shared::WorkerFunction on_execute() override { return nullptr; }

    [[nodiscard]] shared::ReleaseFunction on_released() noexcept override {
        std::weak_ptr<FfiBridge> weak = weak_from_this();
        return [weak] {
            if (auto self = weak.lock()) self->release_plugin();
        };
    }

    [[nodiscard]] shared::ErrorHandler on_error() override {
        std::string lib_name = m_lib_name;
        return [lib_name = std::move(lib_name)](std::exception_ptr eptr) {
            try {
                std::rethrow_exception(std::move(eptr));
            } catch (const std::exception &ex) {
                std::println(stderr, "[ffi::{}] error: {}", lib_name, ex.what());
            } catch (...) {
                std::println(stderr, "[ffi::{}] unknown error", lib_name);
            }
        };
    }

  private:
    explicit FfiBridge(void *lib) : m_lib{lib} {}

    // ── libffi thunks ─────────────────────────────────────────────────────────
    // user_data is bridge.get() — set when constructing each Closure.

    static void log_thunk(ffi_cif * /*cif*/, void * /*ret*/, void **args, void *user_data) noexcept {
        auto *self    = static_cast<FfiBridge *>(user_data);
        auto  level   = *static_cast<int *>(args[1]);
        auto *msg_ptr = *static_cast<const char **>(args[2]);
        auto  msg_len = *static_cast<std::size_t *>(args[3]);
        std::println(stderr, "[plugin::{}] log({}): {}", self->m_lib_name, level,
                     std::string_view{msg_ptr, msg_len});
    }

    static void schedule_thunk(ffi_cif * /*cif*/, void * /*ret*/,
                                void ** /*args*/, void *user_data) noexcept {
        std::println(stderr, "[plugin::{}] schedule requested",
                     static_cast<FfiBridge *>(user_data)->m_lib_name);
    }

    // ── Platform helpers ──────────────────────────────────────────────────────

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

    // ── Plugin lifecycle ──────────────────────────────────────────────────────

    // Returns an error string on failure, empty string on success.
    [[nodiscard]] std::string resolve_symbols() noexcept {
        m_syms.name         = probe<PluginSymbols::NameFn>("congelado_plugin_name");
        m_syms.version      = probe<PluginSymbols::VersionFn>("congelado_plugin_version");
        m_syms.capabilities = probe<PluginSymbols::CapsFn>("congelado_capabilities");

        if ((m_syms.name == nullptr) || (m_syms.version == nullptr) ||
            (m_syms.capabilities == nullptr))
            return "missing required symbols: congelado_plugin_name / "
                   "congelado_plugin_version / congelado_capabilities";

        m_syms.on_load            = probe<PluginSymbols::OnLoadFn>("congelado_on_load");
        m_syms.on_unload          = probe<PluginSymbols::OnUnloadFn>("congelado_on_unload");
        m_syms.logger_write       = probe<PluginSymbols::LogWriteFn>("congelado_logger_write");
        m_syms.logger_write_error = probe<PluginSymbols::LogWriteErrFn>("congelado_logger_write_error");
        m_syms.protocol_get       = probe<PluginSymbols::ProtoGetFn>("congelado_protocol_get");
        return {};
    }

    [[nodiscard]] const CongeladoConfigView *
    build_config_view(const core::config::PluginConfig *plugin_cfg) noexcept {
        if (plugin_cfg == nullptr) return nullptr;
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
        return &m_cfg_view;
    }

    void discover_caps() noexcept {
        m_caps = m_syms.capabilities();

        if (((m_caps & CONGELADO_CAP_PROTOCOL) != 0) && (m_syms.protocol_get != nullptr)) {
            auto *proto = static_cast<interfaces::IProtocol *>(m_syms.protocol_get());
            if (proto != nullptr)
                m_protocol = std::shared_ptr<interfaces::IProtocol>(
                    proto, [](interfaces::IProtocol *) noexcept {});
        }
    }

    void release_plugin() noexcept {
        if (m_syms.on_unload != nullptr) {
            m_syms.on_unload();
            m_syms = {};
        }
    }

    // ── Members ───────────────────────────────────────────────────────────────

    void *m_lib{nullptr};
    PluginSymbols m_syms{};
    std::string m_lib_name;
    std::uint32_t m_caps{0};
    std::shared_ptr<interfaces::IProtocol> m_protocol;
    std::unique_ptr<Closure> m_log_closure;
    std::unique_ptr<Closure> m_sched_closure;

    std::vector<const char *> m_cfg_keys;
    std::vector<const char *> m_cfg_vals;
    CongeladoConfigView m_cfg_view{};
};

} // export namespace core::ffi

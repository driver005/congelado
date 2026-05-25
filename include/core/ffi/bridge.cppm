module;

#include <ffi.h>
#include <stdio.h>
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

export namespace core::ffi {

// Host-side capability enum — values mirror CongeladoCap in plugin_api.h.
enum class Cap : std::uint32_t {
    Logger    = CONGELADO_CAP_LOGGER,
    Protocol  = CONGELADO_CAP_PROTOCOL,
    Custom    = CONGELADO_CAP_CUSTOM,
    RouterCtx = CONGELADO_CAP_ROUTER_CTX,
};

struct LoadError {
    std::string detail;
};

// FfiBridge: RAII wrapper around one loaded CongeladoPlugin*.
//
// Loading:
//   1. dlopen the .so
//   2. Probe "congelado_get_plugin" (direct cast, signature known) → CongeladoPlugin*
//   3. Build libffi closures for CongeladoHostCallbacks (log, schedule)
//   4. Call plugin->on_load(self, &callbacks, cfg)
//   5. Probe capabilities via get_capability(self, cap_id)
//
// Dispatch:
//   All interface calls go through cached cap vtable pointers (m_logger_cap etc.).
//   Always null-checked — never segfaults on missing capability.
class FfiBridge : public shared::HandlerBase,
                  public interfaces::ILogger,
                  public std::enable_shared_from_this<FfiBridge> {
  public:
    [[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
    load(const std::filesystem::path &path,
         const core::config::PluginConfig *plugin_cfg = nullptr,
         void *router_ctx = nullptr) {
        void *lib =
#if defined(_WIN32)
            static_cast<void *>(LoadLibraryA(path.string().c_str()));
#else
            dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
        if (!lib)
            return std::unexpected(LoadError{std::format("dlopen failed: {}", path.string())});

        auto bridge = std::shared_ptr<FfiBridge>(new FfiBridge{});
        bridge->m_lib = lib;

        auto *get_sym = bridge->probe("congelado_get_plugin");
        auto *destroy_sym = bridge->probe("congelado_destroy_plugin");
        if (!get_sym || !destroy_sym)
            return std::unexpected(LoadError{"missing 'congelado_get_plugin' / 'congelado_destroy_plugin'"});

        bridge->m_destroy_sym = destroy_sym;
        bridge->m_plugin = reinterpret_cast<CongeladoPlugin *(*)()>(get_sym)();
        if (!bridge->m_plugin)
            return std::unexpected(LoadError{"congelado_get_plugin returned null"});

        bridge->m_lib_name = bridge->m_plugin->name;

        // Build libffi closures for host callbacks
        void *log_code   = nullptr;
        void *sched_code = nullptr;
        try {
            auto codes = bridge->make_host_callbacks();
            log_code   = codes.first;
            sched_code = codes.second;
        } catch (const std::exception &ex) {
            return std::unexpected(LoadError{std::format("libffi setup failed: {}", ex.what())});
        }
        CongeladoHostCallbacks callbacks{};
        callbacks.log        = reinterpret_cast<CongeladoLogFn>(log_code);
        callbacks.schedule   = reinterpret_cast<CongeladoScheduleFn>(sched_code);
        callbacks.router_ctx = router_ctx;
        callbacks.ctx        = bridge.get();

        std::vector<const char *> pcv_keys, pcv_vals;
        if (plugin_cfg) {
            for (auto &[k, v] : plugin_cfg->fields) {
                pcv_keys.push_back(k.c_str());
                pcv_vals.push_back(v.c_str());
            }
        }
        CongeladoConfigView pcv{
            plugin_cfg ? pcv_keys.data() : nullptr,
            plugin_cfg ? pcv_vals.data() : nullptr,
            plugin_cfg ? pcv_keys.size() : 0,
        };
        bridge->m_plugin->on_load(bridge->m_plugin->self, &callbacks, plugin_cfg ? &pcv : nullptr);

        bridge->discover_caps();

        return bridge;
    }

    ~FfiBridge() {
        release_plugin();
        if (m_log_closure)
            ffi_closure_free(m_log_closure);
        if (m_sched_closure)
            ffi_closure_free(m_sched_closure);
        if (m_lib) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(m_lib));
#else
            dlclose(m_lib);
#endif
        }
    }

    FfiBridge(const FfiBridge &) = delete;
    FfiBridge &operator=(const FfiBridge &) = delete;

    [[nodiscard]] bool has(Cap cap) const noexcept { return (m_caps & std::to_underlying(cap)) != 0; }

    [[nodiscard]] std::shared_ptr<interfaces::IProtocol> get_protocol() const noexcept { return m_protocol; }

    // Returns the plugin's shared RouterContext* (opaque void*), or nullptr.
    // Callers cast to core::server::RouterContext<Protocol>* for typed access.
    [[nodiscard]] void* get_router_ctx() const noexcept { return m_router_ctx_ptr; }

    // shared::HandlerBase + interfaces::ILogger — name() satisfies both.
    [[nodiscard]] std::string_view name() const noexcept override { return m_lib_name; }

    // Null-guarded on both the cap pointer and the individual function pointers.
    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        if (!m_logger_cap || !m_logger_cap->write)
            return;
        m_logger_cap->write(m_logger_cap->self, static_cast<int>(level), msg.data(), msg.size());
    }

    void error(std::string_view msg) noexcept override {
        if (!m_logger_cap || !m_logger_cap->write_error)
            return;
        m_logger_cap->write_error(m_logger_cap->self, msg.data(), msg.size());
    }

    [[nodiscard]] shared::WorkerFunction on_execute() override { return nullptr; }

    [[nodiscard]] shared::ReleaseFunction on_released() noexcept override {
        std::weak_ptr<FfiBridge> weak = weak_from_this();
        return [weak] {
            if (auto self = weak.lock()) self->release_plugin();
        };
    }

    [[nodiscard]] shared::ErrorHandler on_error() override {
        std::string name = m_lib_name; // copy — lambda must not capture `this` (use-after-free risk)
        return [name = std::move(name)](std::exception_ptr eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception &ex) {
                std::println(stderr, "[ffi::{}] error: {}", name, ex.what());
            } catch (...) {
                std::println(stderr, "[ffi::{}] unknown error", name);
            }
        };
    }

  private:
    FfiBridge() = default;

    void *m_lib = nullptr;
    void *m_destroy_sym = nullptr;
    CongeladoPlugin *m_plugin = nullptr;
    CongeladoLoggerCap *m_logger_cap = nullptr;
    void *m_router_ctx_ptr = nullptr; // opaque RouterContext<Protocol>* from plugin
    std::string m_lib_name;
    std::uint32_t m_caps = 0;
    std::shared_ptr<interfaces::IProtocol> m_protocol;

    ffi_closure *m_log_closure = nullptr;
    void *m_log_fn_code = nullptr;
    ffi_cif m_cif_log{};
    ffi_type *m_log_args[4]{};

    ffi_closure *m_sched_closure = nullptr;
    void *m_sched_fn_code = nullptr;
    ffi_cif m_cif_sched{};
    ffi_type *m_sched_args[1]{};

    void release_plugin() noexcept {
        if (!m_plugin)
            return;
        m_plugin->on_unload(m_plugin->self);
        using DestroyFn = void (*)(CongeladoPlugin *);
        reinterpret_cast<DestroyFn>(m_destroy_sym)(m_plugin);
        m_plugin = nullptr;
        m_logger_cap = nullptr;
    }

    [[nodiscard]] void *probe(const char *sym) const noexcept {
#if defined(_WIN32)
        return reinterpret_cast<void *>(GetProcAddress(static_cast<HMODULE>(m_lib), sym));
#else
        return dlsym(m_lib, sym);
#endif
    }

    static ffi_type *size_ffi_type() noexcept { return sizeof(std::size_t) == 8 ? &ffi_type_uint64 : &ffi_type_uint32; }

    static void log_fn(ffi_cif *, void *, void **args, void *data) noexcept {
        auto *self = static_cast<FfiBridge *>(data);
        // args[0] = ctx (unused — bridge ptr comes from closure user_data 'data')
        int level = *static_cast<int *>(args[1]);
        const char *ptr = *static_cast<const char **>(args[2]);
        std::size_t len = *static_cast<std::size_t *>(args[3]);
        std::println(stderr, "[plugin::{}] log({}): {}", self->m_lib_name, level, std::string_view{ptr, len});
    }

    static void schedule_fn(ffi_cif *, void *, void **, void *data) noexcept {
        auto *self = static_cast<FfiBridge *>(data);
        std::println(stderr, "[plugin::{}] schedule requested", self->m_lib_name);
    }

    // Returns {log_fn_code, sched_fn_code}
    [[nodiscard]] std::pair<void *, void *> make_host_callbacks() {
        m_log_args[0] = &ffi_type_pointer;
        m_log_args[1] = &ffi_type_sint;
        m_log_args[2] = &ffi_type_pointer;
        m_log_args[3] = size_ffi_type();
        if (ffi_prep_cif(&m_cif_log, FFI_DEFAULT_ABI, 4, &ffi_type_void, m_log_args) != FFI_OK)
            throw std::runtime_error("ffi_prep_cif(log) failed");
        m_log_closure = static_cast<ffi_closure *>(ffi_closure_alloc(sizeof(ffi_closure), &m_log_fn_code));
        if (!m_log_closure)
            throw std::runtime_error("ffi_closure_alloc(log) failed");
        if (ffi_prep_closure_loc(m_log_closure, &m_cif_log, log_fn, this, m_log_fn_code) != FFI_OK)
            throw std::runtime_error("ffi_prep_closure_loc(log) failed");

        m_sched_args[0] = &ffi_type_pointer;
        if (ffi_prep_cif(&m_cif_sched, FFI_DEFAULT_ABI, 1, &ffi_type_void, m_sched_args) != FFI_OK)
            throw std::runtime_error("ffi_prep_cif(sched) failed");
        m_sched_closure = static_cast<ffi_closure *>(ffi_closure_alloc(sizeof(ffi_closure), &m_sched_fn_code));
        if (!m_sched_closure)
            throw std::runtime_error("ffi_closure_alloc(sched) failed");
        if (ffi_prep_closure_loc(m_sched_closure, &m_cif_sched, schedule_fn, this, m_sched_fn_code) != FFI_OK)
            throw std::runtime_error("ffi_prep_closure_loc(sched) failed");

        return {m_log_fn_code, m_sched_fn_code};
    }

    void discover_caps() {
        if (!m_plugin)
            return;

        // Logger capability
        auto *lc = static_cast<CongeladoLoggerCap *>(m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_LOGGER));
        if (lc) {
            m_logger_cap = lc;
            m_caps |= std::to_underlying(Cap::Logger);
        }

        // Protocol capability — placeholder: plugin returns interfaces::IProtocol* as void*
        auto *proto_raw = m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_PROTOCOL);
        if (proto_raw) {
            auto *proto = static_cast<interfaces::IProtocol *>(proto_raw);
            m_protocol = std::shared_ptr<interfaces::IProtocol>(proto, [](interfaces::IProtocol *) {});
            m_caps |= std::to_underlying(Cap::Protocol);
        }

        // RouterCtx capability — plugin returns RouterContext<Protocol>* as void*
        auto *rctx = m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_ROUTER_CTX);
        if (rctx) {
            m_router_ctx_ptr = rctx;
            m_caps |= std::to_underlying(Cap::RouterCtx);
        }
    }
};

} // namespace core::ffi

module;

#include <any>
#include <congelado/abi.h>
#include <dlfcn.h>
#include <filesystem>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

export module core_plugin:plugin_ref;

import std;

export namespace core::plugin::types {

class PluginRef {
  public:
    struct DlDeleter {
        /**
         * @brief Closes the dlopen handle it owns — the deleter for `unique_ptr<void, DlDeleter>`.
         * @warning This is the only thing standing between a loaded plugin and a leaked
         * `.so` mapping. Skip calling this (e.g. by leaking the unique_ptr) and the shared
         * library never gets dlclose'd — no cap, that's a real handle leak.
         * @param handle the raw dlopen handle to close; no-op if null.
         */
        void operator()(void *handle) const {
            if (handle != nullptr) {
                ::dlclose(handle);
            }
        }
    };

    /// @brief Default-constructs an empty PluginRef with no handle and no loaded symbols yet.
    PluginRef() = default;
    /**
     * @brief Builds a PluginRef around an already-opened dlopen handle.
     * @note Stashes `file_path` straight into `m_data["path"]` so it's available as a symbol
     * lookup down the line — lowkey doubling `m_data` as both a symbol table and metadata bag.
     * @param handle the owning dlopen handle (closed automatically via DlDeleter on destruction).
     * @param file_path filesystem path the shared library was loaded from.
     */
    PluginRef(std::unique_ptr<void, DlDeleter> handle, std::string file_path)
        : m_handle(std::move(handle)) {
        // Take ownership of the dlopen handle, then stash the load path into
        // m_data so it's reachable through the same symbol-lookup table later.
        m_data["path"] = std::move(file_path);
    }

    /**
     * @brief Releases the dlopen handle WITHOUT closing it — the process keeps the `.so` mapped
     * for the rest of its lifetime, `dlclose()` never runs.
     * @warning Only call this at true process-exit time, after every live plugin has already had
     * its `on_unload()` run (flushed/torn down at the C++ object level) — this is a workaround
     * for `dlclose()`-time global destructors inside a plugin's statically-linked dependencies
     * (OpenTelemetry's C++ SDK plus its HTTP/protobuf stack, confirmed live) segfaulting during
     * final process teardown. The OS reclaims the mapping on process exit regardless, so skipping
     * the syscall here trades a harmless, standard "leak everything at exit" for a crash. Never
     * call this on a plugin that might get closed and reopened later (hot-reload) — the handle is
     * gone for good afterward, no way to dlclose or reopen it through this `PluginRef` again.
     */
    void leak_handle() noexcept { static_cast<void>(m_handle.release()); }

    // ── Symbol descriptor types ─────────────────────────────────────────

    enum class SymbolKind : std::uint8_t {
        FUNCTION,  // single function pointer — void*(*)()
        STRING_FN, // const char*(*)() — string getter
        UINT32,    // uint32_t(*)()
        SIZE_T,    // size_t(*)()
        ARRAY,     // data pointer + count pair (auto-loads "{name}" + "{name}_count")
    };

    struct SymbolInfo {
        std::string_view m_name;
        SymbolKind m_kind;
    };

    // ── Shared symbol list (ordering is significant for index-based lookups) ─
    static constexpr SymbolInfo SHARED_SYMBOLS[] = {
        {.m_name = "congelado_init", .m_kind = SymbolKind::FUNCTION},      // index 0
        {.m_name = "congelado_type", .m_kind = SymbolKind::STRING_FN},     // index 1
        {.m_name = "congelado_on_unload", .m_kind = SymbolKind::FUNCTION},   // index 2
        {.m_name = "congelado_on_ready", .m_kind = SymbolKind::FUNCTION},    // index 3
        {.m_name = "congelado_on_shutdown", .m_kind = SymbolKind::FUNCTION}, // index 4
    };

    // helper to return the name for a given position in the SHARED_SYMBOLS array
    /**
     * @brief Gets the symbol name at a given index in `SHARED_SYMBOLS`.
     * @param symbol_index position into the `SHARED_SYMBOLS` array (see the index comments
     * on each entry — 0 is `congelado_init`, 1 is `congelado_type`, and so on).
     * @return the symbol name at that index, or an empty string_view if out of range — this
     * never throws, an out-of-bounds index is just a W-less empty result, not a crash.
     */
    [[nodiscard]] static std::string_view shared_symbol_name(std::size_t symbol_index) noexcept {
        constexpr std::size_t SYMBOL_COUNT = sizeof(SHARED_SYMBOLS) / sizeof(SHARED_SYMBOLS[0]);
        return symbol_index < SYMBOL_COUNT
                   ? SHARED_SYMBOLS[symbol_index].m_name
                   : std::string_view{}; // FIXME(clang-tidy): unchecked operator[], consider .at();
                                         // non-constant array index
    }

    // ── Plugin-specific symbol lists ─────────────────────────────────────
    static constexpr SymbolInfo PLUGIN_SYMBOLS[] = {
        {.m_name = "congelado_plugin_name", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_plugin_version", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_plugin_author", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_plugin_description", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_capabilities", .m_kind = SymbolKind::UINT32},
        {.m_name = "congelado_unique_type", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_requires", .m_kind = SymbolKind::ARRAY},
        {.m_name = "congelado_load_before_types", .m_kind = SymbolKind::ARRAY},
        {.m_name = "congelado_call", .m_kind = SymbolKind::FUNCTION},
    };

    static constexpr SymbolInfo WORKER_SYMBOLS[] = {
        {.m_name = "congelado_worker_type", .m_kind = SymbolKind::STRING_FN},
        {.m_name = "congelado_worker_execute", .m_kind = SymbolKind::FUNCTION},
        {.m_name = "congelado_call", .m_kind = SymbolKind::FUNCTION},
    };

    // ── Data members ────────────────────────────────────────────────────

    std::unique_ptr<void, DlDeleter> m_handle;
    std::unordered_map<std::string, std::any> m_data;
};


[[nodiscard]] inline bool is_shared_lib(const std::filesystem::path &file_path) {
    auto ext = file_path.extension().string();
#ifdef _WIN32
    return ext == ".dll";
#elifdef __APPLE__
    return ext == ".dylib";
#else
    return ext == ".so";
#endif
}

} // namespace core::plugin::types

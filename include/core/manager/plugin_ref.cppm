module;

#include <congelado/abi.h>
#include <any>
#include <memory>
#include <unordered_map>
#include <vector>
#include <span>
#include <filesystem>
#include <dlfcn.h>

export module core_plugin:plugin_ref;

import std;

export namespace core::plugin::types {

class PluginRef {
  public:
    PluginRef() = default;
    PluginRef(std::unique_ptr<void, DlDeleter> handle, std::string file_path) {
        m_handle = std::move(handle);
        m_data["path"] = std::move(file_path);
    }

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

    // ── Shared symbol list (ordering is significant for index-based lookups) ─
    static constexpr SymbolInfo shared_symbols[] = {
        {"congelado_init", SymbolKind::FUNCTION},              // index 0
        {"congelado_type", SymbolKind::STRING_FN},             // index 1
        {"congelado_on_unload", SymbolKind::FUNCTION},  // index 2
        {"congelado_on_ready", SymbolKind::FUNCTION},   // index 3
    };

    // helper to return the name for a given position in the shared_symbols array
    [[nodiscard]] static std::string_view shared_symbol_name(std::size_t idx) noexcept {
        constexpr std::size_t N = sizeof(shared_symbols) / sizeof(shared_symbols[0]);
        return idx < N ? shared_symbols[idx].name : std::string_view{};
    }

    // ── Plugin-specific symbol lists ─────────────────────────────────────
    static constexpr SymbolInfo plugin_symbols[] = {
        {"congelado_plugin_name", SymbolKind::STRING_FN},
        {"congelado_plugin_version", SymbolKind::STRING_FN},
        {"congelado_plugin_author", SymbolKind::STRING_FN},
        {"congelado_plugin_description", SymbolKind::STRING_FN},
        {"congelado_capabilities", SymbolKind::UINT32},
        {"congelado_unique_type", SymbolKind::STRING_FN},
        {"congelado_requires", SymbolKind::ARRAY},
        {"congelado_load_before_types", SymbolKind::ARRAY},
        {"congelado_logger_write", SymbolKind::FUNCTION},
        {"congelado_logger_write_error", SymbolKind::FUNCTION},
        {"congelado_protocol_get", SymbolKind::FUNCTION},
        {"congelado_storage_get", SymbolKind::FUNCTION},
    };

    static constexpr SymbolInfo worker_symbols[] = {
        {"congelado_worker_type", SymbolKind::STRING_FN},
        {"congelado_worker_execute", SymbolKind::FUNCTION},
        {"congelado_logger_write", SymbolKind::FUNCTION},
        {"congelado_logger_write_error", SymbolKind::FUNCTION},
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

} // namespace core::plugin::types

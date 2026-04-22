export module core_plugin:handle;

import std;

extern "C" {
enum PluginKind : std::uint32_t { PLUGIN_AUTONOMOUS = 0, PLUGIN_REACTIVE = 1 };
struct OpaqueEvent {
    const char *topic;
    const char *payload_json;
};
struct HostAPI {
    void (*log)(const char *, const char *);
};
struct PluginVTable {
    std::uint32_t abi_major, abi_minor;
    PluginKind kind;
    void (*on_load)(void *, const HostAPI *);
    void (*on_unload)(void *);
    void (*on_execute)(void *); // ← replaces on_event dispatch loop
    void (*on_event)(void *, const OpaqueEvent *);
    const char *(*name)(void *);
    const char *(*version)(void *);
    const char *(*author)(void *);
    const char *(*description)(void *);
    const char **(*subscriptions)(void *);
    const char **(*dependencies)(void *);
    const char *last_error;
};
using CreatePluginFn = PluginVTable *(*)(void **);
using DestroyPluginFn = void (*)(PluginVTable *, void *);
}

export namespace core::plugin {

struct Event {
    std::string topic;
    std::string payload_json;
};

enum class LoadErrorKind { NotFound, BadABI, MissingSymbol, InitFailed, CycleDetected };
struct LoadError {
    LoadErrorKind kind;
    std::string detail;
    [[nodiscard]] std::string message() const {
        using enum LoadErrorKind;
        auto tag = [&] {
            switch (kind) {
            case NotFound:
                return "not_found";
            case BadABI:
                return "bad_abi";
            case MissingSymbol:
                return "missing_symbol";
            case InitFailed:
                return "init_failed";
            case CycleDetected:
                return "cycle_detected";
            }
            std::unreachable();
        }();
        return std::format("[{}] {}", tag, detail);
    }
};

template <typename T>
concept AnyPlugin = requires(T t, const Event &ev) {
    { t.name() } -> std::convertible_to<std::string_view>;
    { t.version() } -> std::convertible_to<std::string_view>;
};

class PluginHandle {
    struct Deleter {
        DestroyPluginFn destroy;
        void *lib;
        void operator()(PluginVTable *vt) const noexcept;
    };
    using VTablePtr = std::unique_ptr<PluginVTable, Deleter>;

    VTablePtr vtable_;
    void *self_ = nullptr;

  public:
    PluginHandle() = default;
    PluginHandle(PluginVTable *vt, void *self, DestroyPluginFn destroy, void *lib)
        : vtable_{vt, Deleter{destroy, lib}}, self_{self} {}

    PluginHandle(PluginHandle &&) noexcept = default;
    PluginHandle &operator=(PluginHandle &&) noexcept = default;

    [[nodiscard]] bool valid() const noexcept { return vtable_ != nullptr; }
    explicit operator bool() const noexcept { return valid(); }

    // ── Metadata (deducing this) ──────────────────────────────────────────────
    [[nodiscard]] auto name(this auto &&s) noexcept { return std::string_view{s.vtable_->name(s.self_)}; }
    [[nodiscard]] auto version(this auto &&s) noexcept { return std::string_view{s.vtable_->version(s.self_)}; }
    [[nodiscard]] auto author(this auto &&s) noexcept { return std::string_view{s.vtable_->author(s.self_)}; }
    [[nodiscard]] auto description(this auto &&s) noexcept { return std::string_view{s.vtable_->description(s.self_)}; }

    // ── Kind ──────────────────────────────────────────────────────────────────
    [[nodiscard]] PluginKind kind() const noexcept { return vtable_->kind; }

    [[nodiscard]] std::vector<std::string_view> subscriptions() const noexcept {
        return to_vec(vtable_->subscriptions(self_));
    }
    [[nodiscard]] std::vector<std::string_view> dependencies() const noexcept {
        return to_vec(vtable_->dependencies(self_));
    }

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<void, std::string> on_load(const HostAPI &host) {
        vtable_->last_error = nullptr;
        vtable_->on_load(self_, &host);
        if (vtable_->last_error)
            return std::unexpected(std::string{vtable_->last_error});
        return {};
    }

    void on_unload() { vtable_->on_unload(self_); }

    // Autonomous: drives the plugin's own loop
    void execute() { vtable_->on_execute(self_); }

    // Reactive: pushes one event into the plugin's vtable handler
    void on_event(const Event &ev) {
        OpaqueEvent oe{ev.topic.c_str(), ev.payload_json.c_str()};
        vtable_->on_event(self_, &oe);
    }

  private:
    static std::vector<std::string_view> to_vec(const char **arr) noexcept {
        if (!arr)
            return {};
        std::vector<std::string_view> out;
        for (; *arr; ++arr)
            out.emplace_back(*arr);
        return out;
    }
};

static_assert(AnyPlugin<PluginHandle>);

} // namespace core::plugin

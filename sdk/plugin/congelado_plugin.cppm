export module congelado_plugin;

import std;

export namespace congelado {

// C++ view of the host callback table. Constructed by CONGELADO_PLUGIN from C ABI arguments.
// Valid only for the duration of on_load(). Do not store.
class HostCallbacks {
  public:
    using LogFn = void (*)(void *ctx, int level, const char *msg, std::size_t len);
    using SchedFn = void (*)(void *ctx);

    HostCallbacks(LogFn log, SchedFn sched, void *router_ctx,
                  void *controller_ctx, void *leverager_ctx, void *ctx) noexcept
        : m_log{log}, m_sched{sched}, m_router_ctx{router_ctx},
          m_controller_ctx{controller_ctx}, m_leverager_ctx{leverager_ctx}, m_ctx{ctx} {}

    void log(int level, std::string_view msg) const noexcept {
        if (m_log != nullptr) {
            m_log(m_ctx, level, msg.data(), msg.size());
        }
    }
    void schedule() const noexcept {
        if (m_sched != nullptr) {
            m_sched(m_ctx);
        }
    }
    // Typed accessors — C ABI carries void* across the dlopen boundary; cast happens here.
    template <typename T> [[nodiscard]] T *router_ctx()     const noexcept { return static_cast<T *>(m_router_ctx); }
    template <typename T> [[nodiscard]] T *controller_ctx() const noexcept { return static_cast<T *>(m_controller_ctx); }
    template <typename T> [[nodiscard]] T *leverager_ctx()  const noexcept { return static_cast<T *>(m_leverager_ctx); }

  private:
    LogFn  m_log{nullptr};
    SchedFn m_sched{nullptr};
    void *m_router_ctx{nullptr};
    void *m_controller_ctx{nullptr};
    void *m_leverager_ctx{nullptr};
    void *m_ctx{nullptr};
};

// Read-only view of the plugin's [plugins.name] config section.
// Valid only for the duration of on_load(). Do not store.
class ConfigView {
  public:
    ConfigView(const char *const *keys, const char *const *values, std::size_t count) noexcept
        : m_keys{keys}, m_values{values}, m_count{count} {}

    [[nodiscard]] std::size_t size() const noexcept { return m_count; }
    [[nodiscard]] bool empty() const noexcept { return m_count == 0; }

    [[nodiscard]] std::optional<std::string_view> get(std::string_view key) const noexcept {
        for (std::size_t i = 0; i < m_count; ++i) {
            if (std::string_view{m_keys[i]} == key) {
                return std::string_view{m_values[i]};
            }
        }
        return std::nullopt;
    }

    template <typename Callback>
    void for_each(Callback &&callback) const noexcept {
        for (std::size_t i = 0; i < m_count; ++i) {
            std::forward<Callback>(callback)(std::string_view{m_keys[i]},
                                             std::string_view{m_values[i]});
        }
    }

  private:
    const char *const *m_keys{nullptr};
    const char *const *m_values{nullptr};
    std::size_t m_count{0};
};

// C++ base class for plugin authors.
// Inherit this, override virtual methods, drop CONGELADO_PLUGIN(T) at the bottom of your .cc.
class Plugin {
  public:
    Plugin() noexcept = default;
    virtual ~Plugin() = default;
    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin &operator=(Plugin &&) = delete;

    // IMPORTANT: returned string_view::data() is used as a raw const char*.
    // Implementations MUST return a view into a string literal or stable member.
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_version() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t capabilities() const noexcept { return 0; }

    // Returns a type tag for uniqueness enforcement. Empty string = not unique.
    // Only one plugin with a given tag loads; the first in config order wins.
    // MUST return a view into a string literal.
    [[nodiscard]] virtual std::string_view get_unique_type() const noexcept { return {}; }

    // Returns the names of plugins that must be loaded before this plugin.
    // Each name must exactly match get_name() of the required plugin.
    // MUST return a span over a static array of string literals.
    [[nodiscard]] virtual std::span<const std::string_view> get_requires() const noexcept {
        return {};
    }

    // Returns unique-type tags this plugin must be loaded before.
    // Any plugin whose get_unique_type() matches a returned tag will be sorted after this plugin.
    // MUST return a span over a static array of string literals.
    [[nodiscard]] virtual std::span<const std::string_view> get_load_before_types() const noexcept {
        return {};
    }

    virtual void on_load(HostCallbacks const & /*host*/, ConfigView const & /*cfg*/) {}
    virtual void on_unload() {}
    virtual void logger_write(int /*level*/, std::string_view /*msg*/) noexcept {}
    virtual void *protocol_get() noexcept { return nullptr; }
    // Returns interfaces::IDatabase* cast to void*. Override with CONGELADO_CAP_STORAGE.
    virtual void *storage_get() noexcept { return nullptr; }
};

} // namespace congelado

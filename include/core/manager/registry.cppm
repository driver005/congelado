export module core_plugin:registry;

import std;
import shared;
import :handle;
import :handler;
import :loader;

export namespace core::plugin {

[[nodiscard]]
std::expected<std::vector<std::string>, LoadError>
topo_sort(std::span<const std::pair<std::string, std::vector<std::string>>> edges) {
    std::unordered_map<std::string, int> in_degree;
    std::unordered_map<std::string, std::vector<std::string>> succs;
    for (auto &[node, deps] : edges) {
        in_degree.try_emplace(node, 0);
        for (auto &d : deps) {
            succs[d].push_back(node);
            ++in_degree[node];
        }
    }
    std::queue<std::string> ready;
    for (auto &[n, deg] : in_degree)
        if (deg == 0)
            ready.push(n);
    std::vector<std::string> order;
    while (!ready.empty()) {
        auto n = std::move(ready.front());
        ready.pop();
        for (auto &s : succs[n])
            if (--in_degree[s] == 0)
                ready.push(s);
        order.push_back(std::move(n));
    }
    if (order.size() != in_degree.size())
        return std::unexpected(LoadError{LoadErrorKind::CycleDetected, "dependency graph has a cycle"});
    return order;
}

template <shared::HandlerController TController>
class Registry {
    struct Slot {
        PluginEntry entry;
        std::uint32_t id;
    };

    TController &controller_;
    std::unordered_map<std::string, Slot> plugins_;
    std::vector<std::string> load_order_;

    static std::uint32_t make_id(const std::string &name) {
        return static_cast<std::uint32_t>(std::hash<std::string>{}(name));
    }

  public:
    explicit Registry(TController &controller) : controller_{controller} {}

    ~Registry() {
        for (auto &name : load_order_ | std::views::reverse)
            if (auto it = plugins_.find(name); it != plugins_.end())
                controller_.release(it->second.id);
    }

    [[nodiscard]]
    std::expected<void, LoadError> load(const std::filesystem::path &path) {
        auto result = plugin::load(path);
        if (!result)
            return std::unexpected(result.error());

        auto name = std::string{result->name()};

        for (auto dep : result->dependencies())
            if (!plugins_.contains(std::string{dep}))
                return std::unexpected(LoadError{LoadErrorKind::InitFailed,
                                                 std::format("'{}' requires '{}' which is not loaded", name, dep)});

        const auto kind_str = result->kind() == PLUGIN_REACTIVE ? "reactive" : "autonomous";
        std::println("[registry] loaded '{}' v{}  [{}]  kind={}", result->name(), result->version(), result->author(),
                     kind_str);

        auto entry = make_entry(std::move(*result));
        auto id = make_id(name);

        // create() routes through HandlerBase — same for both variants
        std::visit([&](auto &p) { p.create(controller_); }, entry);
        controller_.schedule(id);

        plugins_.emplace(name, Slot{std::move(entry), id});
        load_order_.push_back(std::move(name));
        return {};
    }

    void unload(std::string_view name) {
        auto it = plugins_.find(std::string{name});
        if (it == plugins_.end()) {
            std::println("[registry] '{}' not found", name);
            return;
        }
        controller_.release(it->second.id);
        std::erase(load_order_, std::string{name});
        plugins_.erase(it);
        std::println("[registry] unloaded '{}'", name);
    }

    // publish — only ReactivePlugins receive events; AutonomousPlugins are skipped
    void publish(Event ev) {
        for (auto &[_, slot] : plugins_) {
            if (auto *rp = std::get_if<ReactivePlugin>(&slot.entry)) {
                auto subs = rp->subscriptions();
                bool wildcard = subs.empty();
                bool match = std::ranges::contains(subs, std::string_view{ev.topic});
                if (wildcard || match)
                    rp->post(ev);
            }
            // AutonomousPlugin: silently skipped — it doesn't consume external events
        }
    }

    template <std::convertible_to<std::string> Payload>
    void publish(std::string topic, Payload &&payload) {
        publish(Event{std::move(topic), std::forward<Payload>(payload)});
    }

    void list() const {
        std::println("\n┌─ {} plugin(s) ─────────────────────────────", plugins_.size());
        for (auto &name : load_order_) {
            auto &slot = plugins_.at(name);
            std::visit(
                [](auto &p) {
                    constexpr bool is_reactive = std::same_as<std::decay_t<decltype(p)>, ReactivePlugin>;
                    std::println("│  {} v{}  [{}]  {}  ({})", p.name(), p.version(), p.author(), p.description(),
                                 is_reactive ? "reactive" : "autonomous");
                },
                slot.entry);
        }
        std::println("└────────────────────────────────────────────\n");
    }
};

template <shared::HandlerController TController>
Registry(TController &) -> Registry<TController>;

} // namespace core::plugin

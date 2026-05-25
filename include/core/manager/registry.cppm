export module core_plugin:registry;

import std;
import shared;
import interfaces;
import core_ffi;
import :handle;
import :handler;
import :loader;

export namespace core::plugin {

// Registry manages FfiBridge lifetimes and their scheduler slots.
// On load it discovers capabilities and exposes interface factories
// (make_logger etc.) for callers to register with their subsystems.
template <shared::HandlerController TController>
class Registry {
    struct Slot {
        PluginHandle bridge;
        std::uint32_t id;
    };

    TController &controller_;
    std::unordered_map<std::string, Slot> slots_;
    std::vector<std::string> load_order_;

    static std::uint32_t make_id(std::string_view name) {
        return static_cast<std::uint32_t>(std::hash<std::string_view>{}(name));
    }

  public:
    explicit Registry(TController &controller) : controller_{controller} {}

    ~Registry() {
        for (auto &name : load_order_ | std::views::reverse)
            if (auto it = slots_.find(name); it != slots_.end())
                controller_.release(it->second.id);
    }

    // Loads a library, registers it with the scheduler, and returns the bridge.
    // Callers inspect bridge->has(Cap::Logger) etc. and register interfaces.
    [[nodiscard]]
    std::expected<PluginHandle, LoadError> load(const std::filesystem::path &path) {
        auto result = plugin::load(path);
        if (!result)
            return std::unexpected(result.error());

        auto &bridge = *result;
        auto  name   = std::string{bridge->name()};

        if (slots_.contains(name))
            return std::unexpected(LoadError{std::format("'{}' already loaded", name)});

        auto id = make_id(name);
        bridge->create(controller_);
        controller_.schedule(id);

        std::println("[registry] loaded '{}'", name);
        slots_.emplace(name, Slot{bridge, id});
        load_order_.push_back(std::move(name));

        return result;
    }

    void unload(std::string_view name) {
        auto it = slots_.find(std::string{name});
        if (it == slots_.end()) {
            std::println("[registry] '{}' not found", name);
            return;
        }
        controller_.release(it->second.id);
        std::erase(load_order_, std::string{name});
        slots_.erase(it);
        std::println("[registry] unloaded '{}'", name);
    }

    [[nodiscard]] PluginHandle find(std::string_view name) const {
        auto it = slots_.find(std::string{name});
        return it != slots_.end() ? it->second.bridge : nullptr;
    }

    void list() const {
        std::println("\n┌─ {} bridge(s) ─────────────────────────────", slots_.size());
        for (auto &name : load_order_) {
            auto &slot = slots_.at(name);
            std::print("│  '{}'  caps:", name);
            if (slot.bridge->has(Cap::Logger))   std::print(" Logger");
            if (slot.bridge->has(Cap::Protocol)) std::print(" Protocol");
            if (slot.bridge->has(Cap::Custom))   std::print(" Custom");
            std::println();
        }
        std::println("└────────────────────────────────────────────\n");
    }
};

template <shared::HandlerController TController>
Registry(TController &) -> Registry<TController>;

} // namespace core::plugin

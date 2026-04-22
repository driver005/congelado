module;

#include <stdio.h>

export module core_plugin:handler;

import std;
import shared;
import :handle;

export namespace core::plugin {


inline void log_error(std::string_view plugin_name, std::exception_ptr eptr) {
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception &ex) {
        std::println(stderr, "[plugin::{}] error: {}", plugin_name, ex.what());
    } catch (...) {
        std::println(stderr, "[plugin::{}] unknown error", plugin_name);
    }
}


class AutonomousPlugin final : public shared::HandlerBase {
  public:
    explicit AutonomousPlugin(PluginHandle handle) : handle_{std::move(handle)} {}

    std::string_view name() const noexcept override { return "AutonomousPlugin"; }

    shared::WorkerFunction on_execute() override {
        return [this] { handle_.execute(); };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this] { handle_.on_unload(); };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) { log_error(handle_.name(), eptr); };
    }

    [[nodiscard]] std::string_view get_name() const noexcept { return handle_.name(); }
    [[nodiscard]] std::string_view version() const noexcept { return handle_.version(); }
    [[nodiscard]] std::string_view author() const noexcept { return handle_.author(); }
    [[nodiscard]] std::string_view description() const noexcept { return handle_.description(); }
    [[nodiscard]] std::vector<std::string_view> dependencies() const noexcept { return handle_.dependencies(); }

  private:
    PluginHandle handle_;
};

class ReactivePlugin final : public shared::HandlerBase {
  public:
    explicit ReactivePlugin(PluginHandle handle) : handle_{std::move(handle)} {}

    // Called by Registry::publish() when a matching event arrives.
    void post(Event ev) {
        {
            std::lock_guard lk{mu_};
            pending_.push(std::move(ev));
        }
        shared::this_handler::shedule();
    }

    std::string_view name() const noexcept override { return "ReactivePlugin"; }

    shared::WorkerFunction on_execute() override {
        return [this] {
            std::optional<Event> ev;
            {
                std::lock_guard lk{mu_};
                if (!pending_.empty()) {
                    ev = std::move(pending_.front());
                    pending_.pop();
                }
                if (!pending_.empty())
                    shared::this_handler::shedule(); // more work remains
            }
            if (ev)
                handle_.on_event(*ev);
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this] {
            // drain before unload — plugin sees every event
            std::lock_guard lk{mu_};
            while (!pending_.empty()) {
                handle_.on_event(pending_.front());
                pending_.pop();
            }
            handle_.on_unload();
        };
    }

    shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) { log_error(handle_.name(), eptr); };
    }

    [[nodiscard]] std::string_view get_name() const noexcept { return handle_.name(); }
    [[nodiscard]] std::string_view version() const noexcept { return handle_.version(); }
    [[nodiscard]] std::string_view author() const noexcept { return handle_.author(); }
    [[nodiscard]] std::string_view description() const noexcept { return handle_.description(); }
    [[nodiscard]] std::vector<std::string_view> subscriptions() const noexcept { return handle_.subscriptions(); }
    [[nodiscard]] std::vector<std::string_view> dependencies() const noexcept { return handle_.dependencies(); }

  private:
    PluginHandle handle_;
    std::mutex mu_;
    std::queue<Event> pending_;
};

using PluginEntry = std::variant<AutonomousPlugin, ReactivePlugin>;
[[nodiscard]] inline std::string_view entry_name(const PluginEntry &e) noexcept {
    return std::visit([](auto &p) { return p.name(); }, e);
}


[[nodiscard]] PluginEntry make_entry(PluginHandle handle) {
    if (handle.kind() == PluginKind::PLUGIN_REACTIVE)
        return PluginEntry{std::in_place_type<ReactivePlugin>, std::move(handle)};
    return PluginEntry{std::in_place_type<AutonomousPlugin>, std::move(handle)};
}

} // namespace core::plugin

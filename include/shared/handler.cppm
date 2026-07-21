export module shared:handler;

import std;

export namespace shared {

using WorkerFunction = std::function<void()>;
using ReleaseFunction = std::function<void()>;
using ErrorHandler = std::function<void(std::exception_ptr)>;

template <typename T>
concept HandlerTemplate = requires(T instance) {
    { instance.schedule() } -> std::same_as<void>;
    { instance.deschedule() } -> std::same_as<void>;
    { instance.release() } -> std::same_as<void>;
};

class HandlerInterface {
  public:
    /**
     * @brief Virtual dtor, default's all it needs — polymorphic controllers clean up fine
     * through the base ptr, no extra teardown motion required.
     */
    virtual ~HandlerInterface() = default;
    HandlerInterface() = default;
    HandlerInterface(const HandlerInterface &) = delete;
    HandlerInterface &operator=(const HandlerInterface &) = delete;
    HandlerInterface(HandlerInterface &&) = delete;
    HandlerInterface &operator=(HandlerInterface &&) = delete;

    /**
     * @brief Puts the handler identified by `identifier` on the run queue so it actually gets its turn.
     * @param identifier the handler's id, whatever the concrete controller uses to track it.
     */
    virtual void schedule(std::uint32_t identifier) = 0;
    /**
     * @brief Pulls the handler identified by `identifier` off the run queue — parks it, doesn't kill it.
     * Still around, just not getting motion right now.
     * @param identifier the handler's id to deschedule.
     */
    virtual void deschedule(std::uint32_t identifier) = 0;
    /**
     * @brief Tears the handler identified by `identifier` down for good. After this call it's gone, not
     * just paused — no coming back from a release.
     * @param identifier the handler's id to release.
     */
    virtual void release(std::uint32_t identifier) = 0;
};

template <typename T, typename... OptionalArgs>
concept HandlerController =
    requires(T controller, std::uint32_t identifier) {
        { controller.schedule(identifier) } -> std::same_as<void>;
        { controller.deschedule(identifier) } -> std::same_as<void>;
        { controller.release(identifier) } -> std::same_as<void>;
    } && requires(T controller, std::string_view name, shared::WorkerFunction work, shared::ReleaseFunction release,
                  shared::ErrorHandler error, OptionalArgs... opt_args) {
        { controller.create(name, work, release, error, opt_args...) } -> HandlerTemplate;
    };


class HandlerBase;

template <typename T, typename... Args>
concept ExecutionPattern = requires(HandlerBase &handler, Args &&...args) {
    { T::install(handler, std::forward<Args>(args)...) };
};

class HandlerBase {
  public:
    /**
     * @brief Virtual dtor, default's fine — same deal as HandlerInterface, base ptr cleanup
     * covers it, no extra motion needed.
     */
    virtual ~HandlerBase() = default;
    HandlerBase() = default;
    HandlerBase(const HandlerBase &) = delete;
    HandlerBase &operator=(const HandlerBase &) = delete;
    HandlerBase(HandlerBase &&) = delete;
    HandlerBase &operator=(HandlerBase &&) = delete;

    /**
     * @brief Identifies this handler by name — whatever the concrete implementer calls itself,
     * gets threaded straight into `TController::create()` as the handler's registered name.
     * @return the handler's name.
     */
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    /**
     * @brief Bet — wires this handler's execute/release/error hooks up to a real controller and
     * hands back a live, schedulable handler object. This is the whole point of the class:
     * `HandlerBase` is just a hook bundle until `create()` binds it to a `TController`.
     * @tparam TController the controller type doing the binding, gotta satisfy
     * `HandlerController`.
     * @tparam Args extra args forwarded straight through to `TController::create()`.
     * @param controller the controller that actually spins up the runtime handler object.
     * @param args extra args forwarded straight through to `controller.create()`.
     * @return whatever `HandlerTemplate`-conforming object `controller.create()` hands back.
     */
    template <HandlerController TController, typename... Args>
    auto create(TController &controller, Args &&...args) -> HandlerTemplate auto {
        return controller.create(get_name(), on_execute(), on_released(), on_error(), std::forward<Args>(args)...);
    }

    /**
     * @brief The actual work this handler runs once scheduled — every concrete handler's gotta
     * define this, no default here, it's the whole vibe of the class.
     * @return the callable that gets invoked to do the real work.
     */
    virtual WorkerFunction on_execute() = 0;

    /**
     * @brief Optional cleanup hook that fires after this handler wraps up. Default's a flat
     * no-op (nullptr) — lowkey most handlers don't need one, so only override this if there's
     * real cleanup motion to run.
     * @return the release callback, or nullptr if this handler doesn't need post-run cleanup.
     */
    virtual ReleaseFunction on_released() noexcept { return nullptr; }

    /**
     * @brief Optional hook for exceptions that blow up mid-execution. Default's nullptr, which
     * means "let it propagate" — not "swallow it quietly," don't get it twisted.
     * @return the error handler, or nullptr if this handler doesn't override error handling.
     */
    virtual ErrorHandler on_error() { return nullptr; }
};

} // namespace shared


export namespace shared::this_handler {

// FIXME(clang-tidy): cppcoreguidelines-avoid-non-const-global-variables — this thread_local slot
// is the whole point (it's rebound elsewhere to attach/detach the calling thread's handler
// context), so it can't be made const without breaking that binding.
thread_local inline HandlerInterface *current = nullptr;

// FIXME(clang-tidy): cppcoreguidelines-avoid-non-const-global-variables — mutated alongside
// `current` whenever a thread's handler context is (re)bound; can't be const for the same reason.
thread_local inline std::uint32_t current_id = std::numeric_limits<std::uint32_t>::max();

inline void shedule() {
    // Guard clause — nothing bound to this thread means there's no handler to schedule,
    // so throw loud instead of quietly no-oping.
    if (current == nullptr) {
        throw std::runtime_error(
            std::format("No current contract context for scheduling in thread `{}`", std::this_thread::get_id()));
    }

    // Context's live — hand off to the bound handler's real schedule call.
    current->schedule(current_id);
}

inline void deschedule() {
    // Same guard as shedule() — no bound handler on this thread, nothing to deschedule.
    if (current == nullptr) {
        throw std::runtime_error(
            std::format("No current contract context for descheduling in thread `{}`", std::this_thread::get_id()));
    }

    // Bet, forward straight to the bound handler's deschedule.
    current->deschedule(current_id);
}

inline void release() {
    // Guard clause: can't release a handler that isn't bound to this thread.
    if (current == nullptr) {
        throw std::runtime_error(
            std::format("No current contract context for releaseing in thread `{}`", std::this_thread::get_id()));
    }

    // All good — tear it down for real through the bound handler.
    current->release(current_id);
}

} // namespace shared::this_handler

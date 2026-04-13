export module shared:handler;

import std;

export namespace shared {

using WorkerFunction = std::function<void()>;
using ReleaseFunction = std::function<void()>;
using ErrorHandler = std::function<void(std::exception_ptr)>;

class HandlerInterface {
  public:
    virtual ~HandlerInterface() = default;

    virtual void schedule(std::uint32_t id) = 0;
    virtual void deschedule(std::uint32_t id) = 0;
    virtual void release(std::uint32_t id) = 0;
};

class HandlerBase;

template <typename T, typename... Args>
concept ExecutionPattern = requires(HandlerBase &handler, Args &&...args) {
    { T::install(handler, std::forward<Args>(args)...) };
};

class HandlerBase {
  public:
    virtual ~HandlerBase() = default;

    template <typename TPattern, typename... Args>
        requires ExecutionPattern<TPattern, Args...>
    auto plug_into(Args &&...args) {
        return TPattern::install(*this, std::forward<Args>(args)...);
    }

    virtual WorkerFunction on_execute() = 0;

    virtual ReleaseFunction on_released() noexcept { return nullptr; }

    virtual ErrorHandler on_error() { return nullptr; }
};

} // namespace shared


export namespace shared::this_handler {

thread_local inline HandlerInterface *current = nullptr;

thread_local inline std::uint32_t current_id = std::numeric_limits<std::uint32_t>::max();

inline void shedule() {
    if (!current)
        throw std::runtime_error("No current contract context for scheduling");

    current->schedule(current_id);
}

inline void deschedule() {
    if (!current)
        throw std::runtime_error("No current contract context for descheduling");

    current->deschedule(current_id);
}

inline void release() {
    if (!current)
        throw std::runtime_error("No current contract context for releasing");

    current->release(current_id);
}

} // namespace shared::this_handler

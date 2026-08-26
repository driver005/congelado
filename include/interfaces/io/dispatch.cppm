export module interfaces:io_dispatch;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :io_request;
import :io_response;

export namespace interfaces::io {

/**
 * @brief Mutates a response into its final state. Only ever invoked by the protocol layer
 * (`Session::complete_response()`), and only after it has re-verified the target stream is
 * still alive — a handler's async continuation never gets a live `IResponse&` directly, it
 * only ever builds one of these self-contained closures.
 */
using ResponseWriter = std::move_only_function<void(IResponse&)>;

/**
 * @brief Handed to a pending handler's continuation so it can eventually publish its result.
 * Calling it hands the writer off to the protocol layer, which will invoke it against the
 * live response once it has re-proven the target stream still exists.
 */
using ResponseCompleter = std::function<void(ResponseWriter)>;

/**
 * @brief What a dispatched handler hands back — either it finished synchronously (`res` is
 * already final, safe to send right now), or its real completion is still pending and will
 * arrive later, possibly from a different call stack entirely (a queued DB write, etc).
 * @warning Default-constructed == ready. This is deliberate: every existing void-returning
 * handler is synchronous by definition (see `DispatchFunction`'s adapter), so the "nothing
 * happened, everything's fine" state has to be `ready()`, not `pending()`.
 */
class DispatchResult
{
public:
    /**
     * @brief The shape of a `pending()` result's continuation-starter — invoked immediately
     * (same call stack) by the protocol layer with a `ResponseCompleter` the handler's real
     * async continuation must eventually call, exactly once, to publish its result.
     */
    using OnReady = std::move_only_function<void(ResponseCompleter)>;

    /**
     * @brief Default ctor — ready() by construction, see the class warning.
     */
    DispatchResult() noexcept = default;
    /**
     * @brief Move ctor — the only way to relocate one of these, `OnReady` isn't copyable.
     */
    DispatchResult(DispatchResult&&) noexcept = default;
    /**
     * @brief Move assignment, matches the move ctor.
     */
    DispatchResult& operator=(DispatchResult&&) noexcept = default;
    /**
     * @brief Deleted — `OnReady` (`std::move_only_function`) isn't copyable, so neither is
     * this.
     */
    DispatchResult(const DispatchResult&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    DispatchResult& operator=(const DispatchResult&) = delete;
    /**
     * @brief Default dtor — nothing to release beyond what `OnReady`'s own dtor already
     * handles.
     */
    ~DispatchResult() = default;

    /**
     * @brief Builds a ready result — the handler finished synchronously, `res` is already
     * final.
     * @return a ready `DispatchResult`.
     */
    [[nodiscard]] static DispatchResult ready() noexcept
    {
        return {};
    }

    /**
     * @brief Builds a pending result — the handler's real completion happens later.
     * @param on_ready invoked immediately by whoever calls `subscribe()`, handed a
     * `ResponseCompleter` the eventual async continuation must call exactly once.
     * @return a pending `DispatchResult`.
     */
    [[nodiscard]] static DispatchResult pending(OnReady on_ready)
    {
        DispatchResult result;
        result.m_on_ready = std::move(on_ready);
        return result;
    }

    /**
     * @brief Whether this result is ready right now (as opposed to pending).
     * @return true if ready() — `res` is final and safe to send immediately.
     */
    [[nodiscard]] bool is_ready() const noexcept
    {
        return !m_on_ready;
    }

    /**
     * @brief Kicks off the pending continuation, handing it `complete` to call once it's
     * actually done. No-op if this result is already `ready()`.
     * @param complete the completer the eventual async continuation must invoke exactly once.
     */
    void subscribe(ResponseCompleter complete) &&
    {
        if (!m_on_ready) {
            return;
        }
        auto on_ready = std::move(m_on_ready);
        m_on_ready = nullptr;
        on_ready(std::move(complete));
    }

private:
    OnReady m_on_ready{};
};

/**
 * @brief Type-erased handler callable — wraps either a legacy `void(IRequest&, IResponse&)`
 * handler (auto-adapted into always returning `DispatchResult::ready()`) or a real
 * `DispatchResult(IRequest&, IResponse&)` handler, behind one uniform call signature.
 * @warning This is what lets `HandlerFn`/`ReceiveDispatchFn` change shape without breaking
 * every existing void-returning route lambda across the codebase — construction from either
 * kind of callable just works, no call-site changes needed unless a handler actually wants to
 * return `pending()`.
 */
class DispatchFunction
{
public:
    /**
     * @brief Default ctor — empty, calls to `operator()` would be on an empty
     * `std::function` (same failure mode as an empty `std::function` always has).
     */
    DispatchFunction() noexcept = default;

    /**
     * @brief Null ctor — same empty state as the default ctor, just spelled `nullptr` at the
     * call site (mirrors `std::function`'s own `nullptr_t` ctor, since this wraps one).
     */
    DispatchFunction(std::nullptr_t) noexcept {} // NOLINT(google-explicit-constructor)

    /**
     * @brief Converting ctor from any callable matching `(IRequest&, IResponse&)` — a
     * void-returning one gets wrapped so it always reports `DispatchResult::ready()`; a
     * `DispatchResult`-returning one is stored as-is.
     * @tparam Fn the callable type being wrapped.
     * @param fn the callable to wrap.
     */
    template<typename Fn>
        requires(
            !std::same_as<std::remove_cvref_t<Fn>, DispatchFunction> &&
            std::invocable<std::remove_cvref_t<Fn>&, IRequest&, IResponse&>
        )
    DispatchFunction(Fn&& fn) // NOLINT(google-explicit-constructor)
        :
        m_fn{wrap(std::forward<Fn>(fn))}
    {
    }

    /**
     * @brief Runs the wrapped handler.
     * @param req the inbound request.
     * @param res the response to populate — for a `ready()` result, already final by the time
     * this returns; for a `pending()` result, still at whatever default the caller seeded it
     * with.
     * @return the handler's dispatch result.
     */
    DispatchResult operator()(IRequest& req, IResponse& res) const
    {
        return m_fn(req, res);
    }

    /**
     * @brief Whether this wraps an actual handler.
     * @return true if non-empty.
     */
    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_fn);
    }

private:
    using Erased = std::function<DispatchResult(IRequest&, IResponse&)>;

    template<typename Fn>
    static Erased wrap(Fn&& fn)
    {
        using Target = std::remove_cvref_t<Fn>;
        using Result = std::invoke_result_t<Target&, IRequest&, IResponse&>;
        if constexpr (std::is_void_v<Result>) {
            // Legacy void handler — synchronous by definition, always ready().
            return Erased{
                [target = Target{std::forward<Fn>(fn)}](
                    IRequest& req, IResponse& res
                ) mutable -> DispatchResult {
                    target(req, res);
                    return DispatchResult::ready();
                }
            };
        } else {
            static_assert(
                std::convertible_to<Result, DispatchResult>,
                "a handler must return void or interfaces::io::DispatchResult"
            );
            return Erased{
                [target = Target{std::forward<Fn>(fn)}](
                    IRequest& req, IResponse& res
                ) mutable -> DispatchResult {
                    return target(req, res);
                }
            };
        }
    }

    Erased m_fn;
};

} // namespace interfaces::io

#ifdef CONGELADO_TEST
namespace interfaces::io::tests {
using namespace boost::ut;

suite<"DispatchResult"> dispatch_result_suite = [] {
    "default-constructed and ready() are both immediately ready"_test = [] {
        DispatchResult default_result;
        auto ready_result = DispatchResult::ready();

        expect(default_result.is_ready());
        expect(ready_result.is_ready());
    };

    "pending() is not ready until subscribed"_test = [] {
        auto result = DispatchResult::pending([](ResponseCompleter) {});

        expect(not result.is_ready());
    };

    "subscribe() on a pending result invokes on_ready synchronously with the completer"_test = [] {
        bool on_ready_called = false;
        auto result = DispatchResult::pending([&on_ready_called](ResponseCompleter completer) {
            on_ready_called = true;
            expect(static_cast<bool>(completer));
        });

        std::move(result).subscribe([](ResponseWriter) {});

        expect(on_ready_called);
    };

    "subscribe() on an already-ready result is a no-op"_test = [] {
        bool completer_called = false;
        auto result = DispatchResult::ready();

        std::move(result).subscribe([&completer_called](ResponseWriter) {
            completer_called = true;
        });

        expect(not completer_called);
    };
};

suite<"DispatchFunction"> dispatch_function_suite = [] {
    "default-constructed and nullptr-constructed are both empty"_test = [] {
        DispatchFunction default_fn;
        DispatchFunction null_fn{nullptr};

        expect(not static_cast<bool>(default_fn));
        expect(not static_cast<bool>(null_fn));
    };

    "wrapping a void handler always reports ready()"_test = [] {
        bool called = false;
        DispatchFunction dispatch{[&called](IRequest&, IResponse&) {
            called = true;
        }};
        IRequest req;
        IResponse res;

        expect(static_cast<bool>(dispatch));
        auto result = dispatch(req, res);

        expect(called);
        expect(result.is_ready());
    };

    "wrapping a DispatchResult-returning handler passes the result through"_test = [] {
        DispatchFunction dispatch{[](IRequest&, IResponse&) {
            return DispatchResult::pending([](ResponseCompleter) {});
        }};
        IRequest req;
        IResponse res;

        auto result = dispatch(req, res);

        expect(not result.is_ready());
    };
};

} // namespace interfaces::io::tests
#endif

export module core_router:middleware;

import std;
import interfaces;

export namespace core::router {

template <std::uint8_t MiddlewareSize>
class Middleware {
  public:
    /**
     * @brief Sets up an empty middleware chain.
     */
    constexpr Middleware() = default;

    /**
     * @brief Appends `middleware` to the end of the chain.
     * @param middleware the middleware function to append.
     * @warning The overflow message text says "MaxHandlerSize" — copy-pasted from `Handler`'s
     * error, has nothing to do with `MiddlewareSize`. Message is straight up wrong, no cap, don't
     * trust it for debugging, trust the actual bound (`MiddlewareSize`).
     * @throws std::runtime_error if the chain is already at `MiddlewareSize`.
     */
    constexpr void add_middleware(interfaces::MiddlewareFn middleware) {
        // chain's already maxed out — refuse rather than overrun the backing array
        if (m_middleware_index >= MiddlewareSize) {
            throw std::runtime_error("MiddlewareSize cannot be greater than MaxHandlerSize due to "
                                     "offerflow limitations, please check "
                                     "your routes");
        }

        // tack it onto the end, in registration order — bet, that order is execution order later
        m_middleware[m_middleware_index++] = middleware;  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    // constexpr void
    // push_middleware(const std::array<interfaces::MiddlewareFn , MiddlewareSize>
    // &middleware) noexcept {
    //     if (m_middleware_index + middleware.size() <= MiddlewareSize) {
    //         std::ranges::copy(m_middleware, end());
    //         m_middleware_index += middleware.size();
    //     }
    // }

    /**
     * @brief Runs the middleware slice `[offset, offset + length)` against `req`/`res`, chained
     * via continuation-passing — the real entry point into the middleware pipeline for a matched
     * route.
     * @param req the request being processed, forwarded to every middleware in the slice.
     * @param res the response being built, forwarded to every middleware in the slice.
     * @param offset index of the first middleware to run.
     * @param length how many middlewares to run, starting at `offset`.
     * @note Execution order is strictly ascending by index within the slice — each middleware only
     * advances by calling its `next` callback, so ordering here is 100% caller-controlled by how
     * `offset`/`length` were computed upstream. Get that math wrong and you run the wrong slice, no
     * cap.
     */
    void execute(interfaces::io::IRequest &req, interfaces::io::IResponse &res, std::uint8_t offset,
                 std::uint8_t length) const noexcept {
        run_step(req, res, offset, offset + length);
    }

    /**
     * @brief Gets a mutable iterator to the start of the active (populated) range.
     * @return iterator to the first stored middleware.
     */
    constexpr auto begin() noexcept { return m_middleware.begin(); }
    /**
     * @brief Gets a mutable iterator to the end of the active (populated) range.
     * @return iterator one past the last stored middleware — not the array's physical end.
     */
    constexpr auto end() noexcept { return m_middleware.begin() + m_middleware_index; }

    /**
     * @brief Gets a const iterator to the start of the active (populated) range.
     * @return iterator to the first stored middleware.
     */
    [[nodiscard]] constexpr auto begin() const noexcept { return m_middleware.begin(); }
    /**
     * @brief Gets a const iterator to the end of the active (populated) range.
     * @return iterator one past the last stored middleware — not the array's physical end.
     */
    [[nodiscard]] constexpr auto end() const noexcept {
        return m_middleware.begin() + m_middleware_index;
    }

    /**
     * @brief Gets how many middlewares are currently in the chain.
     * @return the middleware count.
     */
    [[nodiscard]] constexpr const std::uint8_t &get_size() const noexcept {
        return m_middleware_index;
    }
    /**
     * @brief Gets the backing middleware array.
     * @return reference to the fixed-size middleware array.
     */
    [[nodiscard]] constexpr const std::array<interfaces::MiddlewareFn, MiddlewareSize> &
    get_middlewares() const noexcept {
        return m_middleware;
    }

  private:
    /**
     * @brief Recursively runs the middleware at `offset`, handing it a `next` continuation that
     * advances to `offset + 1` when called.
     * @param req the request being processed.
     * @param res the response being built.
     * @param offset the current middleware index to run.
     * @param border the exclusive upper bound of the slice — recursion stops once `offset` reaches
     * this.
     * @warning This is textbook continuation-passing middleware dispatch: if a middleware never
     * calls its `next` callback, the chain just stops right there, silently — no error, no
     * fallthrough to the handler. That's the classic "forgot to call next()" L, and it's on every
     * middleware author to get right since nothing here enforces it. Also captures `this` by
     * reference in the `next` lambda, so the `Middleware` object needs to stay alive for the whole
     * recursive descent.
     */
    void run_step(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                  std::uint8_t offset, std::uint8_t border) const noexcept {
        // once offset reaches border the slice is exhausted — recursion just stops here,
        // no fallthrough to anything else
        if (offset < border) {
            const auto CURRENT_MW = m_middleware[offset];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            // hand the current middleware a `next` that, when called, recurses one step
            // further into the slice — if the middleware never calls it, the chain dies
            // silently right here
            CURRENT_MW(req, res,
                       [&](interfaces::io::IRequest &next_req,
                           interfaces::io::IResponse &next_res) noexcept {
                           this->run_step(next_req, next_res, offset + 1, border);
                       });
        }
    }

    std::array<interfaces::MiddlewareFn, MiddlewareSize> m_middleware{};
    std::uint8_t m_middleware_index{0};
};

} // namespace core::router

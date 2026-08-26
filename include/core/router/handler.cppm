module;
#include <cstddef>
export module core_router:handler;

import std;
import interfaces;
import :consts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::router {

template<std::uint8_t MaxHandlerSize = 8>
class Handler
{
public:
    /**
     * @brief Sets up an empty handler table with the mask initialized to `HANDLER_MASK` (every
     * method lane reads as "unset").
     */
    constexpr Handler() = default;

    /**
     * @brief Registers `handler` for `method`, packing its slot index into an 8-bit lane of
     * `m_handler_mask` keyed by the method's underlying value.
     * @param method the HTTP method to bind — its `std::to_underlying()` value times 8 picks
     * the bit-shift for this method's lane in the mask.
     * @param handler the handler function to store.
     * @note Each method gets one byte-wide lane in the 64-bit mask, so this scheme tops out at
     * 8 distinct method lanes, bet — a method whose underlying value pushes the shift past 63
     * bits is out of scope here, no bounds check on that shift.
     * @throws std::runtime_error if `m_handler_index` has already hit `MaxHandlerSize` (table
     * full), or if `method`'s lane in the mask is already claimed (duplicate handler).
     */
    constexpr void
    add_handler(const interfaces::io::types::Method& method, interfaces::HandlerFn handler)
    {
        // table's full, no room to append another handler — bail before touching the mask
        if (m_handler_index >= MaxHandlerSize) {
            throw std::runtime_error(
                "HandlerSize cannot be greater than MaxHandlerSize due to "
                "offerflow limitations, please check "
                "your routes"
            );
        }

        // each method owns its own byte-wide lane in the mask, picked by its underlying value
        const std::uint8_t SHIFT = std::to_underlying(method) * 8;

        // lane already claimed (not the 0xFF "unset" sentinel) means a dupe registration —
        // cooked
        if (((m_handler_mask >> SHIFT) & 0xFF) != 0xFF) {
            throw std::runtime_error("Handler for method already exists");
        }

        // clear this method's lane, then pack in the slot index we're about to store the
        // handler at — mask and array stay in sync this way
        m_handler_mask &= ~(0xFFULL << SHIFT);
        m_handler_mask |= (static_cast<std::uint64_t>(m_handler_index) << SHIFT);
        m_handler[m_handler_index++] = std::move(
            handler
        ); // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    /**
     * @brief Looks up the handler registered for `method`.
     * @param method the HTTP method to look up.
     * @return the matching handler function, or `nullptr` if `method`'s lane reads as unset
     * (`0xFF`).
     */
    [[nodiscard]] constexpr interfaces::HandlerFn
    find(interfaces::io::types::Method method) const noexcept
    {
        const std::uint8_t IDX = (m_handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return IDX != 0xFF ? m_handler[IDX] : nullptr; // FIXME(clang-tidy): unchecked operator[],
                                                       // consider .at(); non-constant array index
    }

    /**
     * @brief Gets how many handlers are currently registered.
     * @return the handler count.
     */
    [[nodiscard]] constexpr const std::uint8_t& get_size() const noexcept
    {
        return m_handler_index;
    }

    /**
     * @brief Gets the raw packed method-to-slot mask.
     * @return the handler mask.
     */
    [[nodiscard]] constexpr const std::size_t& get_mask() const noexcept
    {
        return m_handler_mask;
    }

    /**
     * @brief Gets the backing handler array.
     * @return reference to the fixed-size handler array.
     */
    [[nodiscard]] constexpr const std::array<interfaces::HandlerFn, MaxHandlerSize>&
    get_handler() const noexcept
    {
        return m_handler;
    }

private:
    std::array<interfaces::HandlerFn, MaxHandlerSize> m_handler{};
    std::size_t m_handler_mask{HANDLER_MASK};
    std::uint8_t m_handler_index{0};
};

template<std::size_t MaxHandlerSize>
class HandlerPool
{
public:
    /**
     * @brief Sets up an empty handler pool.
     */
    constexpr HandlerPool() = default;

    /**
     * @brief Looks up a handler stored at `idx` plus the offset packed for `method` in
     * `handler_mask`.
     * @param idx base offset into the pool — the start of the owning node's handler slots.
     * @param handler_mask the packed method-to-offset mask (same encoding as
     * `Handler::get_mask()`) for the node being queried.
     * @param method the HTTP method being looked up.
     * @warning No bounds check on `idx + offset` against the pool's actual size — pass a stale
     * or mismatched `idx`/`handler_mask` pair and this reads straight out of bounds. It's
     * `noexcept` too, so there's no safety net catching it, just UB. Don't be careless with
     * this one.
     * @return the matching handler, or `nullptr` if `method`'s lane in `handler_mask` is unset.
     */
    [[nodiscard]] constexpr interfaces::HandlerFn find(
        std::size_t idx, std::size_t handler_mask, interfaces::io::types::Method method
    ) const noexcept
    {
        const std::uint8_t OFFSET = (handler_mask >> (std::to_underlying(method) * 8)) & 0xFF;
        return OFFSET != 0xFF ? m_handler[idx + OFFSET]
                              : nullptr; // FIXME(clang-tidy): unchecked operator[], consider
                                         // .at(); non-constant array index
    }

    /**
     * @brief Appends `handler` to the pool — no reordering, no dedup, just tacks it on at the
     * next free slot, lowkey the simplest op in this file.
     * @param handler the handler function to store.
     * @throws std::runtime_error if the pool is already at `MaxHandlerSize`.
     */
    constexpr void add_handler(interfaces::HandlerFn handler)
    {
        // pool's at capacity — nothing to do but throw, no cap, no silent drop
        if (m_handler_index >= MaxHandlerSize) {
            throw std::runtime_error(
                "HandlerSize cannot be greater than MaxHandlerSize due to "
                "offerflow limitations, please check "
                "your routes"
            );
        }

        // append at the next free slot
        m_handler[m_handler_index++] = std::move(
            handler
        ); // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    /**
     * @brief Gets a mutable iterator to the start of the active (populated) range.
     * @return iterator to the first stored handler.
     */
    constexpr auto begin() noexcept
    {
        return m_handler.begin();
    }

    /**
     * @brief Gets a mutable iterator to the end of the active (populated) range.
     * @return iterator one past the last stored handler — not the array's physical end.
     */
    constexpr auto end() noexcept
    {
        return m_handler.begin() + m_handler_index;
    }

    /**
     * @brief Gets a const iterator to the start of the active (populated) range.
     * @return iterator to the first stored handler.
     */
    [[nodiscard]] constexpr auto begin() const noexcept
    {
        return m_handler.begin();
    }

    /**
     * @brief Gets a const iterator to the end of the active (populated) range.
     * @return iterator one past the last stored handler — not the array's physical end.
     */
    [[nodiscard]] constexpr auto end() const noexcept
    {
        return m_handler.begin() + m_handler_index;
    }

    /**
     * @brief Gets a mutable reference to the active handler count.
     * @warning Hands back a non-const reference, so a caller can stomp the index directly and
     * desync it from what's actually been written into `m_handler` — that's basically an open
     * door to a cooked pool state if someone's not careful with it.
     * @return reference to the handler index/count.
     */
    constexpr std::uint8_t& get_size() noexcept
    {
        return m_handler_index;
    }

private:
    std::array<interfaces::HandlerFn, MaxHandlerSize> m_handler{};
    std::uint8_t m_handler_index{0};
};

} // namespace core::router

#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

// static: internal linkage so this doesn't collide with the same-named test helper other
// core_router partition files (builder.cppm, router.cppm) define in this same tests namespace.
static interfaces::HandlerFn noop_handler()
{
    return [](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) {};
}

suite<"Handler"> handler_suite = [] {
    "starts empty — find returns nullptr for any method"_test = [] {
        Handler<4> table;
        expect(table.get_size() == 0);
        expect(table.find(interfaces::io::types::Method::GET) == nullptr);
    };
    "add_handler registers by method, find retrieves it"_test = [] {
        Handler<4> table;
        table.add_handler(interfaces::io::types::Method::GET, noop_handler());

        expect(table.get_size() == 1);
        expect(table.find(interfaces::io::types::Method::GET) != nullptr);
        expect(table.find(interfaces::io::types::Method::POST) == nullptr);
    };
    "registering the same method twice throws"_test = [] {
        Handler<4> table;
        table.add_handler(interfaces::io::types::Method::GET, noop_handler());
        expect(throws<std::runtime_error>([&] {
            table.add_handler(interfaces::io::types::Method::GET, noop_handler());
        }));
    };
    "add_handler throws once the table is full"_test = [] {
        Handler<1> table;
        table.add_handler(interfaces::io::types::Method::GET, noop_handler());
        expect(throws<std::runtime_error>([&] {
            table.add_handler(interfaces::io::types::Method::POST, noop_handler());
        }));
    };
};

suite<"HandlerPool"> handler_pool_suite = [] {
    "starts empty"_test = [] {
        HandlerPool<4> pool;
        expect(pool.get_size() == 0);
        expect(pool.begin() == pool.end());
    };
    "add_handler appends, find resolves via idx + mask offset"_test = [] {
        HandlerPool<4> pool;
        pool.add_handler(noop_handler());
        pool.add_handler(noop_handler());

        Handler<4> table;
        table.add_handler(interfaces::io::types::Method::GET, noop_handler());

        // Handler's own mask encodes GET at offset 0 into ITS table; reusing that mask against
        // the pool with idx=1 should resolve to slot 1 (idx + offset).
        expect(pool.find(1, table.get_mask(), interfaces::io::types::Method::GET) != nullptr);
        expect(pool.find(1, table.get_mask(), interfaces::io::types::Method::POST) == nullptr);
    };
    "add_handler throws once the pool is full"_test = [] {
        HandlerPool<1> pool;
        pool.add_handler(noop_handler());
        expect(throws<std::runtime_error>([&] {
            pool.add_handler(noop_handler());
        }));
    };

    // find()'s own doc comment flags it: no bounds check on idx + offset against the pool's
    // real size, and the caller gets back whatever's at that slot with zero validation that it
    // was ever actually registered. Here idx + offset (2) stays well within HandlerSize (4) — a
    // real, in-bounds, default-constructed std::function sitting in a slot nobody ever called
    // add_handler() for — so this is safe to exercise directly. The true out-of-bounds case
    // (idx + offset >= HandlerSize) is real UB per the @warning above and is NOT exercised
    // here; that would read past the backing std::array's storage.
    "find on an in-bounds but never-registered slot returns whatever's sitting there, unvalidated"_test =
        [] {
            HandlerPool<4> pool;
            pool.add_handler(noop_handler());
            pool.add_handler(noop_handler());
            // Slots 0 and 1 are now real registered handlers; slots 2 and 3 are still whatever
            // the default std::array<HandlerFn, 4> init left them as (empty std::function, i.e.
            // operator bool() == false) — never touched by add_handler().

            // Craft a mask whose GET lane points at offset 2, which no add_handler() call above
            // ever wrote to. idx=0 + offset=2 = 2, safely inside HandlerSize=4.
            const std::size_t OFFSET_INTO_UNREGISTERED_SLOT = 2;
            const std::size_t MASK =
                ~(0xFFULL << (std::to_underlying(interfaces::io::types::Method::GET) * 8));
            const std::size_t GET_MASK =
                MASK | (OFFSET_INTO_UNREGISTERED_SLOT
                        << (std::to_underlying(interfaces::io::types::Method::GET) * 8));

            const auto RESULT = pool.find(0, GET_MASK, interfaces::io::types::Method::GET);
            // find() has no notion of "was this slot ever registered" — it just reads
            // m_handler[idx
            // + offset] and hands it back. Slot 2 was never assigned, so this is the array's
            // default-constructed std::function: a real, callable-typed value that's empty, not
            // a nullptr and not flagged invalid in any way by find() itself.
            expect(not static_cast<bool>(RESULT));
        };
};

} // namespace core::router::tests
#endif

export module utils_buffering:deleter;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::buffering {

class Deleter
{
public:
    struct Internal
    {
        int m_ref_count{1};
        /**
         * @brief Fires the wrapped action. Pure virtual — every concrete action type plugs its
         * own callable in here, that's the whole point of the type erasure.
         */
        virtual void destroy() noexcept = 0;
        /**
         * @brief Default virtual dtor, keeps deletion through the base pointer safe.
         */
        virtual ~Internal() = default;

        /** @brief Default ctor — ref count starts at 1 via the member initializer above. */
        Internal() = default;
        /** @brief Deleted — this base is only ever handled through `Internal*`, copying would
         * slice. */
        Internal(const Internal&) = delete;
        /** @brief Deleted — same reasoning as the copy ctor. */
        Internal& operator=(const Internal&) = delete;
        /** @brief Deleted — same reasoning as the copy ctor. */
        Internal(Internal&&) = delete;
        /** @brief Deleted — same reasoning as the copy ctor. */
        Internal& operator=(Internal&&) = delete;
    };

    template<std::invocable Action>
    struct ConcreteInternal final : Internal
    {
        Action m_action;

        /**
         * @brief Wraps `action` by move, ref count starts at 1 courtesy of `Internal`'s default
         * member init. Bet.
         * @param action the callable to wrap.
         */
        explicit ConcreteInternal(Action&& action) :
            m_action(std::move(action))
        {
        }

        /**
         * @brief Invokes the wrapped action — this is the payoff, whatever `m_action` does
         * happens right here, no in-between.
         */
        void destroy() noexcept override
        {
            m_action();
        }
    };

    /**
     * @brief Empty deleter, no action attached. `empty()` reads true until you assign a real
     * one in.
     */
    Deleter() :
        m_internal{nullptr}
    {
    }

    /**
     * @brief Wraps `action` in a heap-allocated `ConcreteInternal`, ref count 1, no cap.
     * @tparam Action any invocable.
     * @param action the cleanup callable to own.
     */
    template<std::invocable Action>
    explicit Deleter(Action&& action) :
        m_internal(new ConcreteInternal<Action>(std::forward<Action>(action)))
    {
    }

    /**
     * @brief Copy ctor — doesn't clone the action, just bumps the ref count and shares the same
     * `Internal*`. Cheap on purpose, that's the whole motion of a ref-counted handle.
     * @param other the deleter to share ownership with.
     */
    Deleter(const Deleter& other) noexcept :
        m_internal(other.m_internal)
    {
        if (m_internal != nullptr) {
            ++m_internal->m_ref_count;
        }
    }

    /**
     * @brief Move ctor — steals `other`'s internal pointer outright, no ref-count churn needed.
     * @param other the deleter to pull ownership from, left empty after.
     */
    Deleter(Deleter&& other) noexcept :
        m_internal(std::exchange(other.m_internal, nullptr))
    {
    }

    /**
     * @brief Copy assignment — shares ownership with `other` via a temporary copy, then swaps.
     * @param other the deleter to copy from.
     * @return `*this`, now sharing ownership with `other`.
     */
    Deleter& operator=(const Deleter& other) noexcept
    {
        Deleter temp(other);
        std::swap(m_internal, temp.m_internal);
        return *this;
    }

    /**
     * @brief Move assignment — steals `other`'s internal pointer outright, no ref-count churn
     * needed.
     * @param other the deleter to assign from, left empty after.
     * @return `*this`, now holding whatever `other` had.
     */
    Deleter& operator=(Deleter&& other) noexcept
    {
        std::swap(m_internal, other.m_internal);
        return *this;
    }

    /**
     * @brief Drops this deleter's reference, firing the action if it was the last one standing.
     */
    ~Deleter() noexcept
    {
        release();
    }

    /**
     * @brief Decrements the ref count and, if it hits zero, runs `destroy()` on the wrapped
     * action and frees the `Internal`. Safe to call on an empty deleter — it's a no-op then.
     * @warning Not thread-safe — `m_ref_count` is a plain `int`, no atomics involved. Racing
     * `release()` calls across threads on shared copies of the same `Deleter` is an easy way to
     * double-free or corrupt the count, lowkey a footgun if this ever crosses a thread boundary
     * without external locking.
     */
    void release() noexcept
    {
        // No-op on an empty deleter. Otherwise drop the ref count — lowkey only the caller that
        // brings it to zero actually fires the action and frees the internal state, everyone
        // else just walks away.
        if ((m_internal != nullptr) && --m_internal->m_ref_count == 0) {
            m_internal->destroy();
            delete m_internal;
        }
    }

    /**
     * @brief Checks whether this deleter actually holds an action.
     * @return true if there's no wrapped action, false otherwise.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return m_internal == nullptr;
    }

    /**
     * @brief Grabs the current ref count.
     * @return how many `Deleter` instances share this action right now, 0 if empty.
     */
    [[nodiscard]] int use_count() const noexcept
    {
        return (m_internal != nullptr) ? m_internal->m_ref_count : 0;
    }

private:
    Internal* m_internal;
};

} // namespace utils::buffering

#ifdef CONGELADO_TEST
namespace utils::buffering::tests {
using namespace boost::ut;

suite<"Deleter"> deleter_suite = [] {
    "default-constructed deleter is empty"_test = [] {
        Deleter deleter;
        expect(deleter.empty());
        expect(deleter.use_count() == 0);
    };
    "wrapping an action starts at ref count 1 and fires exactly once on final release"_test = [] {
        int fire_count = 0;
        {
            Deleter deleter([&fire_count] {
                ++fire_count;
            });
            expect(not deleter.empty());
            expect(deleter.use_count() == 1);
            expect(fire_count == 0);
        }
        expect(fire_count == 1);
    };
    "copying shares one action and bumps the ref count"_test = [] {
        int fire_count = 0;
        Deleter first([&fire_count] {
            ++fire_count;
        });
        {
            Deleter second = first;
            expect(first.use_count() == 2);
            expect(second.use_count() == 2);
        }
        expect(fire_count == 0); // second's destruction only dropped the count, didn't fire
        expect(first.use_count() == 1);
    };
    "move steals ownership without firing the action"_test = [] {
        int fire_count = 0;
        Deleter first([&fire_count] {
            ++fire_count;
        });
        Deleter second = std::move(first);

        expect(first.empty());
        expect(not second.empty());
        expect(second.use_count() == 1);
        expect(fire_count == 0);
    };
};

} // namespace utils::buffering::tests
#endif

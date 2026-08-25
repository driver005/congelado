export module io_base_flow;

import std;
import shared;
import io_base_leverage;
import utils_buffering;

export import io_flow_sender;
export import io_flow_receiver;
export import io_flow_socket;

#ifdef CONGELADO_TEST
import boost.ut;
#endif


export namespace io::base::flow {

template <::shared::HandlerController Controller, typename... Ts>
class Flow {
  public:
    /**
     * @brief Builds a Flow bound to a leverager and controller — starts with an empty recipe
     * tuple, nothing's actually instantiated until build() gets called.
     * @param lev the leverager handed to every recipe when it's eventually invoked.
     * @param ctrl the controller handed to every recipe when it's eventually invoked.
     */
    Flow(leverage::Leverager<leverage::Context> &lev, Controller ctrl) : m_leverager(lev), m_controller(ctrl) {}

    /**
     * @brief Builder-chain add — queues up a new `T` recipe (constructed lazily at build() time)
     * and returns a fresh `Flow` whose type pack now includes it. Bet: this doesn't mutate
     * `*this`, it hands back a brand-new `Flow<Controller, Ts..., decltype(recipe)>` instead, so
     * each `.add()` call in a chain changes the static type.
     * @tparam T the flow component type to add, must satisfy `shared::FlowBase` for this
     * Controller/Leverager pair.
     * @tparam Args extra constructor args forwarded to `T`'s constructor at build() time.
     * @param shared the read callback shared with the new component.
     * @param args extra args forwarded straight through to `T`'s constructor.
     * @return a new `Flow` with the recipe for `T` appended to its type pack.
     */
    template <typename T, typename... Args>
        requires ::shared::FlowBase<T, Controller, leverage::Leverager<leverage::Context>>
    auto add(::shared::ReadCallback shared, Args... args) {
        // Stash a lazy recipe — nothing gets built yet, just captures what's needed to build `T`
        // once the leverager/controller are actually available.
        auto recipe = [=](auto &lev, auto &ctrl) { return T{shared, lev, ctrl, args...}; };
        // Hand back a new Flow whose recipe tuple grows by one — the old *this stays untouched.
        return Flow<Controller, Ts..., decltype(recipe)>{m_leverager, m_controller,
                                                         std::tuple_cat(m_recipes, std::make_tuple(recipe))};
    }

    // Build: Triggers instantiation
    /**
     * @brief Actually instantiates every queued recipe, in order, against the stored leverager
     * and controller — this is where all that lazy `.add()` chaining finally turns into real
     * objects. No cap, this is the whole payoff of the builder pattern up top.
     * @return a tuple holding one constructed instance per queued component, same order they
     * were added in.
     */
    auto build() {
        return std::apply([&](auto &...recipes) { return std::make_tuple(recipes(m_leverager, m_controller)...); },
                          m_recipes);
    }

  private:
    // Private constructor for internal builder transitions
    /**
     * @brief Internal ctor used by add() to hand back a new Flow carrying the previous recipes
     * plus the freshly appended one — lowkey just plumbing, not meant to be called directly
     * outside the builder chain.
     * @param lev the leverager to carry forward.
     * @param ctrl the controller to carry forward.
     * @param recipes the concatenated recipe tuple (previous recipes + the new one).
     */
    Flow(leverage::Leverager<leverage::Context> &lev, Controller ctrl, auto recipes)
        : m_leverager(lev), m_controller(ctrl), m_recipes(recipes) {}

    leverage::Leverager<leverage::Context> &m_leverager;  // FIXME(clang-tidy): cppcoreguidelines-avoid-const-or-ref-data-members — switching to a pointer would change nullability/rebinding semantics across the builder chain (add()/build()); not a mechanical fix
    Controller m_controller;
    std::tuple<Ts...> m_recipes = std::make_tuple();
};

} // namespace io::base::flow

#ifdef CONGELADO_TEST
namespace io::base::flow::flow_tests {

// Satisfies shared::HandlerController — id-taking schedule/deschedule/release plus create().
class MockFlowController {
  public:
    void schedule(std::uint32_t) {}
    void deschedule(std::uint32_t) {}
    void release(std::uint32_t) {}

    struct Scheduled {
        void schedule() {}
        void deschedule() {}
        void release() {}
    };

    Scheduled create(std::string_view, shared::WorkerFunction, shared::ReleaseFunction, shared::ErrorHandler) {
        return {};
    }
};

// Satisfies shared::FlowBase<MockFlowComponent, MockFlowController, leverage::Leverager<leverage::Context>> —
// constructible from (ReadCallback&&, Leverager<Context>&, Controller), has on_send(int) -> SendCallback.
// This is exactly the shape `Flow::add<T>()` requires of T.
class MockFlowComponent {
  public:
    MockFlowComponent(shared::ReadCallback &&, leverage::Leverager<leverage::Context> &, MockFlowController) {}
    shared::SendCallback on_send(int) {
        return [](utils::buffering::BufferNode &&) {};
    }
};

// Missing on_send() — should not satisfy FlowBase.
class NotAFlowComponent {
  public:
    NotAFlowComponent(shared::ReadCallback &&, leverage::Leverager<leverage::Context> &, MockFlowController) {}
};

using namespace boost::ut;

suite<"Flow concept gating"> flow_concepts_suite = [] {
    "MockFlowController satisfies shared::HandlerController"_test = [] {
        expect(shared::HandlerController<MockFlowController>);
    };

    "MockFlowComponent satisfies the exact shared::FlowBase shape Flow::add<T>() requires"_test = [] {
        expect((shared::FlowBase<MockFlowComponent, MockFlowController, leverage::Leverager<leverage::Context>>));
    };

    "a component missing on_send() does not satisfy that FlowBase shape"_test = [] {
        expect(!(shared::FlowBase<NotAFlowComponent, MockFlowController, leverage::Leverager<leverage::Context>>));
    };
};

// Flow's constructor/add()/build() are hardcoded (not template-parameterized) to
// `leverage::Leverager<leverage::Context>&` — constructing one spins up a live io_uring ring, the
// same "not unit-testable in isolation" situation `io_base_leverage`'s own test block documents
// for Leverager<Context> itself. There's no injection point here (unlike Receiver/Sender, which
// take a mockable Worker type param), so runtime construction/add()/build() can't be exercised
// without live io_uring — only the compile-time concept gating add<T>() relies on is covered above.

} // namespace io::base::flow::flow_tests
#endif

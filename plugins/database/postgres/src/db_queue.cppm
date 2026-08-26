export module postgres_db_queue;

import connector;
import core_contract;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace postgres {

/**
 * @brief Registers the shared connector's DB-queue Contract, driven by this plugin instead of
 * the host — see plugins/README.md. Kept in its own module TU so the plugin's own `.cc` never
 * imports `connector` directly (that import crashes clang's modules in a plugin entry TU —
 * every other connector-touching plugin dodges it the same way).
 * @note Starts the contract IDLE, not SCHEDULED — the contract is only scheduled when
 * `Connector::enqueue()` transitions the pending queue from empty to non-empty; starting
 * SCHEDULED would make the very first enqueue() try to schedule an already-scheduled contract
 * and abort.
 * @param connector_ctx the host's `connector_ctx` (a `connector::Connector*`, opaque as
 * `void*`).
 * @param contract_group the host's contract group, or nullptr if unavailable.
 * @param contract_registry the host's contract registry, or nullptr if unavailable — the
 * contract is added here so `stop()` can release it before the thread pool joins.
 * @return true if the contract was registered, false if any pointer was null (nothing to do).
 */
bool register_connector_contract(
    void* connector_ctx,
    core::contract::ContractGroup<>* contract_group,
    core::contract::ContractRegistry* contract_registry
)
{
    if (connector_ctx == nullptr || contract_group == nullptr || contract_registry == nullptr) {
        return false;
    }
    auto* shared_connector = static_cast<connector::Connector*>(connector_ctx);
    auto db_contract =
        shared_connector->create(*contract_group, core::contract::ContractState::IDLE);
    shared_connector->set_wake([c = db_contract]() mutable {
        c.schedule();
    });
    contract_registry->add(std::move(db_contract));
    return true;
}

} // namespace postgres

#ifdef CONGELADO_TEST
namespace postgres_db_queue_tests {
using namespace boost::ut;

suite<"postgres::register_connector_contract"> register_connector_contract_suite = [] {
    "all null returns false"_test = [] {
        auto result = postgres::register_connector_contract(nullptr, nullptr, nullptr);

        expect(!result);
    };

    "null connector_ctx returns false and registers nothing"_test = [] {
        core::contract::ContractGroup<> group;
        core::contract::ContractRegistry registry;

        auto result = postgres::register_connector_contract(nullptr, &group, &registry);

        expect(!result);
        expect(registry.empty());
    };

    "null contract_group returns false and registers nothing"_test = [] {
        connector::Connector shared_connector;
        core::contract::ContractRegistry registry;

        auto result = postgres::register_connector_contract(&shared_connector, nullptr, &registry);

        expect(!result);
        expect(registry.empty());
    };

    "null contract_registry returns false and registers nothing"_test = [] {
        connector::Connector shared_connector;
        core::contract::ContractGroup<> group;

        auto result = postgres::register_connector_contract(&shared_connector, &group, nullptr);

        expect(!result);
        // No registry to inspect here — the function must bail out before touching the
        // group/connector at all, which the "all valid" case below confirms happens on success.
    };

    "all valid pointers registers the contract and returns true"_test = [] {
        connector::Connector shared_connector;
        core::contract::ContractGroup<> group;
        core::contract::ContractRegistry registry;

        expect(registry.empty());

        auto result = postgres::register_connector_contract(&shared_connector, &group, &registry);

        expect(result);
        expect(registry.size() == 1);
        expect(!registry.empty());

        // Cleanup — release the contract this test just registered so it doesn't linger.
        registry.release_all();
        expect(registry.empty());
    };
};

} // namespace postgres_db_queue_tests
#endif

export module congelado_heart:context;

import std;
import interfaces;
import io_shared;
import core_router;
import core_contract;
import io_base_leverage;

export namespace congelado::heart {

class AppContext {
  public:
    /**
     * @brief Spins up the app-wide context — router, contract group, and leverager all default
     * to empty, and a single-thread contract thread pool gets emplaced against the contract
     * group right away so there's motion the moment a plugin needs it.
     */
    AppContext() { m_thread_pool.emplace(m_contract_group, 1); }

    /// @brief Gets the shared router context handed to every loading plugin. @return a pointer
    /// to the owned `RouterContext`, never null.
    [[nodiscard]] core::router::RouterContext<> *get_router() noexcept { return &m_router; }

    /// @brief Gets the contract group backing the thread pool. @return a reference to the owned
    /// `ContractGroup`.
    [[nodiscard]] core::contract::ContractGroup<> &get_contract_group() noexcept {
        return m_contract_group;
    }

    /// @brief Gets the leverager shared across plugins for I/O leverage work. @return a
    /// reference to the owned `Leverager`.
    [[nodiscard]] io::base::leverage::Leverager<io::base::leverage::Context> &
    get_leverager() noexcept {
        return m_leverager;
    }

  private:
    core::contract::ContractGroup<> m_contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::optional<core::contract::ContractThreadPool<>> m_thread_pool;

    core::router::RouterContext<> m_router;
};

} // namespace congelado::heart

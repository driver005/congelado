export module core_heart:context;

import std;
import interfaces;
import io_shared;
import core_server;
import core_contract;
import io_base_leverage;

export namespace core::heart {

using Protocol = io::shared::http::Protocol;

class AppContext {
  public:
    AppContext() { m_thread_pool.emplace(m_contract_group, 1); }

    [[nodiscard]] core::server::RouterContext<Protocol> *get_router() noexcept { return &m_router; }

    [[nodiscard]] core::contract::ContractGroup<> &get_contract_group() noexcept {
        return m_contract_group;
    }

    [[nodiscard]] io::base::leverage::Leverager<io::base::leverage::Context> &
    get_leverager() noexcept {
        return m_leverager;
    }

  private:
    core::contract::ContractGroup<> m_contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::optional<core::contract::ContractThreadPool<>> m_thread_pool;

    core::server::RouterContext<Protocol> m_router;
};

} // namespace core::heart

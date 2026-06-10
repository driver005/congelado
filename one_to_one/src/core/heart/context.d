module core.heart.context;
@nogc nothrow:

import io.shared.http.http    : Protocol;
import core.server.builder    : RouterContext;
import core.contracts.contract : ContractGroup, ContractThreadPool;
import io.base.leverage.base  : Leverager;
import io.base.leverage.types : Context;
import util.optional          : Optional;
import util.alloc             : make, dispose;

class AppContext {
  public:
    this() {
        m_contract_group = make!(ContractGroup!())();
        m_leverager      = make!(Leverager!Context)();
        // PORT-NOTE: std::optional<ContractThreadPool<>> m_thread_pool →
        //   Optional!(ContractThreadPool!()) — initialized with 1 thread
        scope auto pool = make!(ContractThreadPool!())(m_contract_group, 1);
        m_thread_pool   = pool;
        m_router        = make!(RouterContext!Protocol)();
    }

    ~this() {
        if (m_thread_pool !is null) dispose(m_thread_pool);
        dispose(m_contract_group);
        dispose(m_leverager);
        dispose(m_router);
    }

    RouterContext!Protocol* get_router() { return m_router; }

    ContractGroup!() get_contract_group() { return m_contract_group; }

    Leverager!Context get_leverager() { return m_leverager; }

  private:
    ContractGroup!()     m_contract_group;
    Leverager!Context    m_leverager;
    ContractThreadPool!() m_thread_pool;

    RouterContext!Protocol m_router;
}

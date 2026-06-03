export module engine:context;

import connector;
import interfaces;
import model;

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
  public:
    void set_db(interfaces::IDatabase *database) noexcept {
        m_db = database;
        m_task_def_connector.set_database(database);
        m_workflow_def_connector.set_database(database);
        m_exec_connector.set_database(database);
    }

    void set_cache(interfaces::ICache *cache) noexcept {
        m_cache = cache;
        m_task_def_connector.set_cache(cache);
        m_workflow_def_connector.set_cache(cache);
        m_exec_connector.set_cache(cache);
    }

    [[nodiscard]] interfaces::IDatabase *get_db()    const noexcept { return m_db; }
    [[nodiscard]] interfaces::ICache    *get_cache() const noexcept { return m_cache; }

    [[nodiscard]] connector::Connector<model::TaskDef>           &get_task_def_connector()     noexcept { return m_task_def_connector; }
    [[nodiscard]] connector::Connector<model::WorkflowDef>       &get_workflow_def_connector() noexcept { return m_workflow_def_connector; }
    [[nodiscard]] connector::Connector<model::WorkflowExecution> &get_exec_connector()         noexcept { return m_exec_connector; }

  private:
    interfaces::IDatabase *m_db{nullptr};
    interfaces::ICache    *m_cache{nullptr};

    connector::Connector<model::TaskDef>           m_task_def_connector;
    connector::Connector<model::WorkflowDef>       m_workflow_def_connector;
    connector::Connector<model::WorkflowExecution> m_exec_connector;
};

} // namespace engine

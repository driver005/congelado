module worker.config;

@nogc nothrow:

// PORT-NOTE: C++ included <toml++/toml.hpp> and used toml::parse_file / toml::table.
// D port stubs from_file() — full TOML parsing deferred to Run 2 (no @nogc TOML lib yet).
// The class shapes are preserved exactly.

class TaskConfig {
  public:
    void set_name(const(char)[] name) { m_name = name; }
    void set_worker_type(const(char)[] worker_type) { m_worker_type = worker_type; }

    const(char)[] get_name() const { return m_name; }
    const(char)[] get_worker_type() const { return m_worker_type; }

  private:
    // PORT-NOTE: C++ used std::string (owning); D uses const(char)[] borrowed view.
    // Lifetime: caller must keep the source string alive.
    const(char)[] m_name;
    const(char)[] m_worker_type;
}

class WorkerConfig {
  public:
    // PORT-NOTE: C++ threw std::runtime_error on parse failure.
    // D cannot throw; returns default-constructed WorkerConfig on failure.
    // Logging via core::logger deferred — wire in Run 2 when logger is stable.
    static WorkerConfig from_file(const(char)[] path) {
        // TODO: wire @nogc TOML parser in Run 2 (toml-d or hand-rolled subset)
        return new WorkerConfig();
    }

    void add_task(TaskConfig task) {
        // PORT-NOTE: @nogc dynamic append — wire util.alloc in Run 2
        m_tasks ~= task;
    }

    void set_engine_url(const(char)[] url) { m_engine_url = url; }
    void set_worker_id(const(char)[] worker_id) { m_worker_id = worker_id; }
    void set_concurrency(uint concurrency) { m_concurrency = concurrency; }

    const(char)[] get_engine_url() const { return m_engine_url; }
    const(char)[] get_worker_id() const { return m_worker_id; }
    uint get_concurrency() const { return m_concurrency; }
    TaskConfig[] get_tasks() const { return cast(TaskConfig[]) m_tasks; }

  private:
    const(char)[] m_engine_url;
    const(char)[] m_worker_id;
    uint m_concurrency = 0;
    TaskConfig[] m_tasks;
}

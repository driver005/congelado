export module congelado_worker:config;

import std;
import serde;

export namespace congelado::worker {

class TaskConfig {
  public:
    /// @brief Default-constructs an empty TaskConfig — fields get filled in by
    /// `serde::Ser::deserialize` (dispatched to the TOML format plugin) or by the setters below.
    TaskConfig() = default;

    /// @brief Sets the task's config-file entry name. @param name the task's name.
    void setName(std::string name) { m_name = std::move(name); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets which worker type handles this task — that's the motion this task actually
    /// dispatches to. @param workerType the worker type string this task dispatches to.
    void setWorkerType(std::string workerType) { m_worker_type = std::move(workerType); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /// @brief Gets the task's config-file entry name. @return the task's name.
    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets which worker type handles this task. @return the worker type string.
    [[nodiscard]] const std::string &getWorkerType() const noexcept { return m_worker_type; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

  private:
    std::string m_name;
    std::string m_worker_type;
};

// engine_url is the only field left genuinely optional (std::optional<std::string> — the
// one optional-primitive FieldConverter this project's serde already has a specialization
// for). Every other field is required: serde::Ser::deserialize errors on a missing required
// field rather than silently defaulting, which is a clearer failure mode for a config
// loader than the previous hand-rolled parser's silent "localhost"/8080 fallbacks — and the
// real worker.toml already sets all of them explicitly.
class WorkerConfig {
  public:
    /// @brief Default-constructs an empty WorkerConfig — real values come from `from_file` or
    /// the setters below.
    WorkerConfig() = default;

    /// @brief Sets the engine URL — the one genuinely optional field in this config.
    /// @param url the engine URL, or `std::nullopt` to leave it unset.
    void setEngineUrl(std::optional<std::string> url) { m_engine_url = std::move(url); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets this worker's identifier. @param workerId the worker's ID string.
    void setWorkerId(std::string workerId) { m_worker_id = std::move(workerId); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets how many tasks this worker runs concurrently. @param concurrency the
    /// concurrency limit.
    void setConcurrency(std::uint32_t concurrency) { m_concurrency = concurrency; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the engine host to connect to. @param host the engine's hostname.
    void setEngineHost(std::string host) { m_engine_host = std::move(host); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the engine port to connect to. @param port the engine's port number.
    void setEnginePort(std::uint32_t port) { m_engine_port = port; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the host this worker's own inbound HTTP server binds to. @param host the
    /// bind host.
    void setBindHost(std::string host) { m_bind_host = std::move(host); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the port this worker's own inbound HTTP server binds to. @param port the
    /// bind port.
    void setBindPort(std::uint32_t port) { m_bind_port = port; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the TLS cert used for the engine connection. @param cert the cert contents.
    void setEngineCert(std::string cert) { m_engine_cert = std::move(cert); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Sets the TLS key used for the engine connection. @param key the key contents.
    void setEngineKey(std::string key) { m_engine_key = std::move(key); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Appends one task config to the list — no cap, this is additive, existing entries
    /// stay put. @param task the task config to add.
    void addTask(TaskConfig task) { m_tasks.push_back(std::move(task)); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Replaces the whole task config list. @param tasks the new set of task configs.
    void setTasks(std::vector<TaskConfig> tasks) { m_tasks = std::move(tasks); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /// @brief Gets the engine URL. @return the engine URL, or empty if never set — the only
    /// field in this config that's genuinely allowed to be missing.
    [[nodiscard]] const std::optional<std::string> &getEngineUrl() const noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        return m_engine_url;
    }
    /// @brief Gets this worker's identifier. @return the worker's ID string.
    [[nodiscard]] const std::string &getWorkerId() const noexcept { return m_worker_id; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the configured concurrency limit. @return how many tasks run at once.
    [[nodiscard]] std::uint32_t getConcurrency() const noexcept { return m_concurrency; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the engine host. @return the engine's hostname.
    [[nodiscard]] const std::string &getEngineHost() const noexcept { return m_engine_host; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the engine port. @return the engine's port number.
    [[nodiscard]] std::uint32_t getEnginePort() const noexcept { return m_engine_port; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the bind host for this worker's own inbound HTTP server. @return the bind
    /// host.
    [[nodiscard]] const std::string &getBindHost() const noexcept { return m_bind_host; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the bind port for this worker's own inbound HTTP server. @return the bind
    /// port.
    [[nodiscard]] std::uint32_t getBindPort() const noexcept { return m_bind_port; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the TLS cert for the engine connection. @return the cert contents.
    [[nodiscard]] const std::string &getEngineCert() const noexcept { return m_engine_cert; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the TLS key for the engine connection. @return the key contents.
    [[nodiscard]] const std::string &getEngineKey() const noexcept { return m_engine_key; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /// @brief Gets the configured task list. @return the task configs, in file order.
    [[nodiscard]] const std::vector<TaskConfig> &getTasks() const noexcept { return m_tasks; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /**
     * @brief Reads and TOML-decodes a `WorkerConfig` straight off disk — this is the real
     * entrypoint worker authors hit at startup, no cap.
     * @note Every field except `engine_url` is required: `serde::Ser::deserialize` errors on a
     * missing required field rather than silently defaulting, which is a clearer failure mode
     * than the old hand-rolled parser's silent `"localhost"`/`8080` fallbacks.
     * @param path filesystem path to the TOML config file.
     * @return the decoded `WorkerConfig` on success, or an error message if the file can't be
     * opened or fails to decode.
     */
    [[nodiscard]] static std::expected<WorkerConfig, std::string>
    from_file(const std::filesystem::path &path);

  private:
    std::optional<std::string> m_engine_url;
    std::string m_worker_id;
    std::uint32_t m_concurrency{0};
    std::string m_engine_host;
    std::uint32_t m_engine_port{0};
    std::string m_bind_host;
    std::uint32_t m_bind_port{0};
    std::string m_engine_cert;
    std::string m_engine_key;
    std::vector<TaskConfig> m_tasks;
};

} // namespace congelado::worker

template <>
struct serde::Serializable<congelado::worker::TaskConfig> {
    static constexpr auto fields() {
        using congelado::worker::TaskConfig;
        return std::tuple{
            serde::FieldDesc<"name", &TaskConfig::getName, &TaskConfig::setName>{},
            serde::FieldDesc<"worker_type", &TaskConfig::getWorkerType, &TaskConfig::setWorkerType>{},
        };
    }
};

template <>
struct serde::Serializable<congelado::worker::WorkerConfig> {
    static constexpr auto fields() {
        using congelado::worker::WorkerConfig;
        return std::tuple{
            serde::FieldDesc<"engine_url", &WorkerConfig::getEngineUrl, &WorkerConfig::setEngineUrl>{},
            serde::FieldDesc<"worker_id", &WorkerConfig::getWorkerId, &WorkerConfig::setWorkerId>{},
            serde::FieldDesc<"concurrency", &WorkerConfig::getConcurrency,
                             &WorkerConfig::setConcurrency>{},
            serde::FieldDesc<"engine_host", &WorkerConfig::getEngineHost,
                             &WorkerConfig::setEngineHost>{},
            serde::FieldDesc<"engine_port", &WorkerConfig::getEnginePort,
                             &WorkerConfig::setEnginePort>{},
            serde::FieldDesc<"bind_host", &WorkerConfig::getBindHost, &WorkerConfig::setBindHost>{},
            serde::FieldDesc<"bind_port", &WorkerConfig::getBindPort, &WorkerConfig::setBindPort>{},
            serde::FieldDesc<"engine_cert", &WorkerConfig::getEngineCert,
                             &WorkerConfig::setEngineCert>{},
            serde::FieldDesc<"engine_key", &WorkerConfig::getEngineKey,
                             &WorkerConfig::setEngineKey>{},
            serde::FieldDesc<"tasks", &WorkerConfig::getTasks, &WorkerConfig::setTasks>{},
        };
    }
};

[[nodiscard]] inline std::expected<congelado::worker::WorkerConfig, std::string>
congelado::worker::WorkerConfig::from_file(const std::filesystem::path &path) {
    // File has to actually open before there's anything to read.
    std::ifstream file{path};
    if (!file) {
        return std::unexpected{std::format("failed to open '{}'", path.string())};
    }
    // Slurp the whole file, then let serde::Ser handle validation/decoding (dispatches to the
    // TOML format plugin, force-loaded before this ever runs — see worker_main.cc) —
    // required-field errors surface from deserialize() itself, not here.
    std::string contents{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    return serde::Ser::deserialize<WorkerConfig>("application/toml", contents);
}

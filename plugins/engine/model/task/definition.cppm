export module model:task_def;

import std;
import :task_status;
import :policies;
import serde;

export namespace model {

class TaskDef {
  public:
    /// @brief Default ctor, bet — SIMPLE type, empty name/worker_type, default retry/timeout
    /// policies, no rate limit.
    TaskDef() = default;

    /// @brief Appends a key to the list of input keys this task expects.
    /// @param key the input key to add.
    void add_input_key(std::string key) { m_input_keys.push_back(std::move(key)); }
    /// @brief Appends a key to the list of output keys this task produces.
    /// @param key the output key to add.
    void add_output_key(std::string key) { m_output_keys.push_back(std::move(key)); }

    /// @brief Sets the task definition's name.
    /// @param name the new name — also the primary key when this def gets persisted.
    void set_name(std::string name) { m_name = std::move(name); }
    /// @brief Sets the task type — this is what decides whether worker_type actually matters,
    /// no cap.
    /// @param type SIMPLE, FORK, JOIN, SWITCH, or SUB_WORKFLOW.
    void set_type(TaskType type) noexcept { m_type = type; }
    /// @brief Sets the worker type this task dispatches to.
    /// @param type the worker type identifier.
    void set_worker_type(std::string type) { m_worker_type = std::move(type); }
    /// @brief Replaces the whole input key list wholesale.
    /// @param input the new set of input keys.
    void set_input_keys(std::vector<std::string> input) { m_input_keys = std::move(input); }
    /// @brief Replaces the whole output key list wholesale.
    /// @param output the new set of output keys.
    void set_output_keys(std::vector<std::string> output) { m_output_keys = std::move(output); }
    /// @brief Sets the retry policy for this task.
    /// @param retry the new retry policy.
    void set_retry(RetryPolicy retry) noexcept { m_retry = retry; }
    /// @brief Sets the timeout policy for this task.
    /// @param timeout the new timeout policy.
    void set_timeout(TimeoutPolicy timeout) noexcept { m_timeout = timeout; }
    /// @brief Sets the (optional) rate-limit policy for this task.
    /// @param rate_limit the new rate-limit policy, or std::nullopt for no limit.
    void set_rate_limit(std::optional<RateLimitPolicy> rate_limit) noexcept {
        m_rate_limit = rate_limit;
    }
    /// @brief Sets the input key whose runtime value names the actual TaskDef to resolve and
    /// run, for TaskType::DYNAMIC defs. Ignored for every other type.
    /// @param param the input key name, or std::nullopt to clear it.
    void set_dynamic_task_param(std::optional<std::string> param) {
        m_dynamic_task_param = std::move(param);
    }
    /// @brief Sets how long a WAIT-typed instance auto-completes after, in milliseconds.
    /// std::nullopt (the default) means the instance waits indefinitely for an external SIGNAL
    /// WorkflowEvent instead of a timer.
    /// @param wait_duration_ms the wait duration, or std::nullopt to wait for a signal.
    void set_wait_duration_ms(std::optional<std::uint32_t> wait_duration_ms) noexcept {
        m_wait_duration_ms = wait_duration_ms;
    }
    /// @brief Sets the isolation domain this task's instances get filtered into — workers
    /// polling with a matching `domain` see only these instances, everyone else's poll skips
    /// them. std::nullopt (the default) means "no domain," visible to any domain-less poll.
    /// @param domain the domain name, or std::nullopt to clear it.
    void set_domain(std::optional<std::string> domain) { m_domain = std::move(domain); }
    /// @brief Sets a JSON-schema document (raw text) input_data gets checked against before a
    /// SIMPLE task's instance is spawned, when enforce_schema is true.
    /// @param schema the schema text, or std::nullopt to clear it.
    void set_input_schema(std::optional<std::string> schema) { m_input_schema = std::move(schema); }
    /// @brief Sets a JSON-schema document (raw text) output_data gets checked against before a
    /// completed instance's result is accepted, when enforce_schema is true.
    /// @param schema the schema text, or std::nullopt to clear it.
    void set_output_schema(std::optional<std::string> schema) {
        m_output_schema = std::move(schema);
    }
    /// @brief Turns schema checking on/off — see SchemaValidator's own docs for exactly what
    /// gets checked (required top-level keys only, not full JSON-schema semantics).
    /// @param enforce true to reject instances that don't satisfy input_schema/output_schema.
    void set_enforce_schema(bool enforce) noexcept { m_enforce_schema = enforce; }
    /// @brief Replaces the whole masked-field list wholesale — these input/output_data keys get
    /// redacted before an execution is ever returned over the API.
    /// @param fields the new set of field names to mask.
    void set_masked_fields(std::vector<std::string> fields) { m_masked_fields = std::move(fields); }

    /// @brief Gets the task definition's name.
    /// @return the configured name.
    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    /// @brief Gets the task type.
    /// @return SIMPLE, FORK, JOIN, SWITCH, or SUB_WORKFLOW.
    [[nodiscard]] TaskType get_type() const noexcept { return m_type; }
    /// @brief Gets the worker type this task dispatches to.
    /// @return the configured worker type identifier.
    [[nodiscard]] const std::string &get_worker_type() const noexcept { return m_worker_type; }
    /// @brief Gets the list of input keys this task expects.
    /// @return the configured input keys.
    [[nodiscard]] const std::vector<std::string> &get_input_keys() const noexcept {
        return m_input_keys;
    }
    /// @brief Gets the list of output keys this task produces.
    /// @return the configured output keys.
    [[nodiscard]] const std::vector<std::string> &get_output_keys() const noexcept {
        return m_output_keys;
    }
    /// @brief Gets the retry policy.
    /// @return the configured retry policy.
    [[nodiscard]] const RetryPolicy &get_retry() const noexcept { return m_retry; }
    /// @brief Gets the timeout policy.
    /// @return the configured timeout policy.
    [[nodiscard]] const TimeoutPolicy &get_timeout() const noexcept { return m_timeout; }
    /// @brief Gets the rate-limit policy, if any — lowkey optional since not every task needs
    /// throttling.
    /// @return the configured rate-limit policy, or std::nullopt if unset.
    [[nodiscard]] const std::optional<RateLimitPolicy> &get_rate_limit() const noexcept {
        return m_rate_limit;
    }
    /// @brief Gets the input key naming the actual TaskDef to resolve at runtime, for
    /// TaskType::DYNAMIC defs.
    /// @return the input key name, or std::nullopt if unset.
    [[nodiscard]] const std::optional<std::string> &get_dynamic_task_param() const noexcept {
        return m_dynamic_task_param;
    }
    /// @brief Gets how long a WAIT-typed instance auto-completes after.
    /// @return the wait duration in milliseconds, or std::nullopt if it waits for a signal
    /// instead.
    [[nodiscard]] const std::optional<std::uint32_t> &get_wait_duration_ms() const noexcept {
        return m_wait_duration_ms;
    }
    /// @brief Gets the isolation domain this task's instances get filtered into.
    /// @return the domain name, or std::nullopt if this task has none.
    [[nodiscard]] const std::optional<std::string> &get_domain() const noexcept { return m_domain; }
    /// @brief Gets the input JSON-schema document, if one's configured.
    /// @return the schema text, or std::nullopt if unset.
    [[nodiscard]] const std::optional<std::string> &get_input_schema() const noexcept {
        return m_input_schema;
    }
    /// @brief Gets the output JSON-schema document, if one's configured.
    /// @return the schema text, or std::nullopt if unset.
    [[nodiscard]] const std::optional<std::string> &get_output_schema() const noexcept {
        return m_output_schema;
    }
    /// @brief Says whether schema checking is turned on for this task.
    /// @return true if input_schema/output_schema get enforced.
    [[nodiscard]] bool get_enforce_schema() const noexcept { return m_enforce_schema; }
    /// @brief Gets the masked-field list.
    /// @return the configured field names to redact, empty if none.
    [[nodiscard]] const std::vector<std::string> &get_masked_fields() const noexcept {
        return m_masked_fields;
    }

    /**
     * @brief Checks the def is internally consistent — non-empty name, worker_type present for
     * SIMPLE tasks, and every nested policy validates clean.
     * @warning Only SIMPLE tasks get the worker_type check. FORK/JOIN/SWITCH/SUB_WORKFLOW get a
     * free pass here even though nothing else in this class enforces they're wired up right —
     * that's on the engine to catch, not this validate(). Don't assume a passing validate()
     * means a SUB_WORKFLOW def actually points anywhere real.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Name's the persistence PK — can't ship a def with nothing to call it.
        if (m_name.empty()) {
            return std::unexpected{"TaskDef name must not be empty"};
        }
        // worker_type only actually matters for SIMPLE tasks, since that's the type that
        // dispatches straight to a worker — everything else gets a pass on this check.
        if (m_type == TaskType::SIMPLE && m_worker_type.empty()) {
            return std::unexpected{"TaskDef worker_type must not be empty for SIMPLE tasks"};
        }
        // Delegate to the nested retry policy's own validate() and bubble its error straight up.
        if (auto result = m_retry.validate(); !result) {
            return result;
        }
        // Same delegation pattern for the timeout policy.
        if (auto result = m_timeout.validate(); !result) {
            return result;
        }
        // Rate limit is optional — only validate it when one's actually configured, lowkey no
        // point checking something that isn't there.
        if (m_rate_limit) {
            if (auto result = m_rate_limit->validate(); !result) {
                return result;
            }
        }
        return {};
    }

  private:
    std::string m_name;
    TaskType m_type{TaskType::SIMPLE};
    std::string m_worker_type;
    std::vector<std::string> m_input_keys;
    std::vector<std::string> m_output_keys;
    RetryPolicy m_retry;
    TimeoutPolicy m_timeout;
    std::optional<RateLimitPolicy> m_rate_limit;
    std::optional<std::string> m_dynamic_task_param;
    std::optional<std::uint32_t> m_wait_duration_ms;
    std::optional<std::string> m_domain;
    std::optional<std::string> m_input_schema;
    std::optional<std::string> m_output_schema;
    bool m_enforce_schema{false};
    std::vector<std::string> m_masked_fields;
};

} // namespace model

template <>
struct serde::Serializable<model::TaskDef> {
    /// @brief The DB table this def gets persisted to — bet, straightforward one-liner.
    /// @return the table name, "task_definitions".
    static constexpr std::string_view table_name() { return "task_definitions"; }
    /**
     * @brief Field-descriptor table wiring TaskDef's columns (name, type, worker_type, keys,
     * policies) to their getters/setters, for serde (de)serialization — name is the PK, no
     * motion needed to keep this synced with the class above, just add a FieldDesc.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"name", &model::TaskDef::get_name, &model::TaskDef::set_name,
                         serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"type", &model::TaskDef::get_type, &model::TaskDef::set_type>{},
            serde::FieldDesc<"worker_type", &model::TaskDef::get_worker_type,
                       &model::TaskDef::set_worker_type>{},
            serde::FieldDesc<"input_keys", &model::TaskDef::get_input_keys,
                       &model::TaskDef::set_input_keys>{},
            serde::FieldDesc<"output_keys", &model::TaskDef::get_output_keys,
                       &model::TaskDef::set_output_keys>{},
            serde::FieldDesc<"retry", &model::TaskDef::get_retry, &model::TaskDef::set_retry>{},
            serde::FieldDesc<"timeout", &model::TaskDef::get_timeout, &model::TaskDef::set_timeout>{},
            serde::FieldDesc<"rate_limit", &model::TaskDef::get_rate_limit,
                       &model::TaskDef::set_rate_limit>{},
            serde::FieldDesc<"dynamic_task_param", &model::TaskDef::get_dynamic_task_param,
                       &model::TaskDef::set_dynamic_task_param>{},
            serde::FieldDesc<"wait_duration_ms", &model::TaskDef::get_wait_duration_ms,
                       &model::TaskDef::set_wait_duration_ms>{},
            serde::FieldDesc<"domain", &model::TaskDef::get_domain, &model::TaskDef::set_domain>{},
            serde::FieldDesc<"input_schema", &model::TaskDef::get_input_schema,
                       &model::TaskDef::set_input_schema>{},
            serde::FieldDesc<"output_schema", &model::TaskDef::get_output_schema,
                       &model::TaskDef::set_output_schema>{},
            serde::FieldDesc<"enforce_schema", &model::TaskDef::get_enforce_schema,
                       &model::TaskDef::set_enforce_schema>{},
            serde::FieldDesc<"masked_fields", &model::TaskDef::get_masked_fields,
                       &model::TaskDef::set_masked_fields>{},
        };
    }
};

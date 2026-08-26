export module worker_orchestrator_local_store;

import std;
import connector;
import model;
import serde;
#ifdef CONGELADO_TEST
import interfaces;
import shared;
import boost.ut;
#endif

export namespace worker_orchestrator {

/// @brief The connector-touching half of the local orchestrator backend, kept in a module TU so
/// the plugin's own `.cc` never imports `connector` directly (that import crashes clang's
/// modules in a plugin entry TU — the engine and the db_query worker dodge it the same way).
/// Every method casts the opaque `connector_ctx` the host injected and runs the same async
/// connector ops the engine's built-in Orchestrator uses, delivering results through the
/// caller's callback. Shares the engine's task store (the same shared Connector), so tasks the
/// engine enqueues are claimable here.
class OrchestratorStore
{
public:
    /// @brief Enqueues a SCHEDULED TaskInstance for `task_type`, delivering its id.
    static void enqueue(
        void* connector_ctx,
        std::string task_type,
        const serde::Value& input,
        std::move_only_function<void(std::optional<std::string>)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(std::nullopt);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name(std::move(task_type));
        instance.set_status(model::TaskStatus::SCHEDULED);
        instance.set_input_data(input);
        auto task_id = std::format("{}", instance.get_task_id());
        conn.insert<model::TaskInstance>(
            instance,
            [task_id = std::move(task_id), callback = std::move(callback)](bool oke) mutable {
                callback(oke ? std::optional<std::string>{std::move(task_id)} : std::nullopt);
            }
        );
    }

    /// @brief Claims the oldest SCHEDULED instance for `worker_type` (optional `domain`), flips
    /// it to IN_PROGRESS, and delivers it serialized as JSON. std::nullopt = empty queue.
    static void claim(
        void* connector_ctx,
        std::string worker_type,
        std::optional<std::string> domain,
        std::move_only_function<void(std::optional<std::string>)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(std::nullopt);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        auto where = std::format(
            "task_instances.status = 'SCHEDULED' AND task_definitions.worker_type = '{}'",
            worker_type
        );
        if (domain) {
            where += std::format(" AND task_definitions.domain = '{}'", *domain);
        }
        auto options =
            connector::QueryOptions{}
                .add_join(
                    "JOIN task_definitions ON task_instances.def_name = task_definitions.name"
                )
                .add_where(where)
                .add_order_by("task_instances.seq");
        conn.find_first<model::TaskInstance>(
            std::move(options),
            [&conn, worker_type, domain](const model::TaskInstance& instance) noexcept {
                if (instance.get_status() != model::TaskStatus::SCHEDULED) {
                    return false;
                }
                bool matches = false;
                conn.find<model::TaskDef>(
                    instance.get_def_name(),
                    [&worker_type, &domain, &matches](std::optional<model::TaskDef> def) noexcept {
                        if (!def || def->get_worker_type() != worker_type) {
                            return;
                        }
                        matches = !domain || def->get_domain() == domain;
                    }
                );
                return matches;
            },
            [](const model::TaskInstance& lhs, const model::TaskInstance& rhs) noexcept {
                return lhs.get_seq() < rhs.get_seq();
            },
            [&conn,
             callback = std::move(callback)](std::optional<model::TaskInstance> found) mutable {
                if (!found) {
                    callback(std::nullopt);
                    return;
                }
                found->set_status(model::TaskStatus::IN_PROGRESS);
                auto claimed = std::move(*found);
                conn.find<model::TaskDef>(
                    claimed.get_def_name(),
                    [&conn, claimed,
                     callback = std::move(callback)](std::optional<model::TaskDef> def) mutable {
                        auto timeout_ms = def ? def->get_timeout().get_timeout_ms() : 30'000U;
                        claimed.set_deadline_at(
                            std::chrono::system_clock::now() + std::chrono::milliseconds{timeout_ms}
                        );
                        conn.update<model::TaskInstance>(
                            claimed, [claimed, callback = std::move(callback)](bool oke) mutable {
                                if (!oke) {
                                    callback(std::nullopt);
                                    return;
                                }
                                auto encoded = serde::Ser::serialize("application/json", claimed);
                                callback(std::string{encoded.begin(), encoded.end()});
                            }
                        );
                    }
                );
            }
        );
    }

    /// @brief Records a worker's result for a claimed task (terminal status + output).
    static void submit_result(
        void* connector_ctx,
        std::string task_id,
        bool success,
        std::unordered_map<std::string, std::string> output,
        std::move_only_function<void(bool)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(false);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        conn.find<model::TaskInstance>(
            std::move(task_id),
            [&conn, success, output = std::move(output),
             callback = std::move(callback)](std::optional<model::TaskInstance> found) mutable {
                if (!found) {
                    callback(false);
                    return;
                }
                found->set_status(
                    success ? model::TaskStatus::COMPLETED : model::TaskStatus::FAILED
                );
                found->set_output_data(std::move(output));
                auto updated = std::move(*found);
                conn.update<model::TaskInstance>(
                    updated, [callback = std::move(callback)](bool oke) mutable {
                        callback(oke);
                    }
                );
            }
        );
    }

    /// @brief Ready-queue depth for `worker_type`.
    static void queue_size(
        void* connector_ctx,
        std::string worker_type,
        std::move_only_function<void(std::size_t)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(0);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        conn.find_all<model::TaskDef>([&conn, worker_type, callback = std::move(callback)](
                                          std::vector<model::TaskDef> defs
                                      ) mutable {
            auto names = std::make_shared<std::unordered_set<std::string>>();
            for (const auto& def: defs) {
                if (def.get_worker_type() == worker_type) {
                    names->insert(def.get_name());
                }
            }
            conn.find_all<model::TaskInstance>([names, callback = std::move(callback)](
                                                   std::vector<model::TaskInstance> instances
                                               ) mutable {
                std::size_t count = 0;
                for (const auto& inst: instances) {
                    if (inst.get_status() == model::TaskStatus::SCHEDULED &&
                        names->contains(inst.get_def_name())) {
                        ++count;
                    }
                }
                callback(count);
            });
        });
    }

    /// @brief Requeues IN_PROGRESS tasks for `worker_type` back to SCHEDULED.
    static void requeue(
        void* connector_ctx,
        std::string worker_type,
        std::move_only_function<void(std::size_t)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(0);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        conn.find_all<model::TaskDef>([&conn, worker_type, callback = std::move(callback)](
                                          std::vector<model::TaskDef> defs
                                      ) mutable {
            auto names = std::make_shared<std::unordered_set<std::string>>();
            for (const auto& def: defs) {
                if (def.get_worker_type() == worker_type) {
                    names->insert(def.get_name());
                }
            }
            conn.find_all<model::TaskInstance>([&conn, names, callback = std::move(callback)](
                                                   std::vector<model::TaskInstance> instances
                                               ) mutable {
                std::size_t count = 0;
                for (auto& inst: instances) {
                    if (inst.get_status() == model::TaskStatus::IN_PROGRESS &&
                        names->contains(inst.get_def_name())) {
                        inst.set_status(model::TaskStatus::SCHEDULED);
                        conn.update<model::TaskInstance>(inst, [](bool) {});
                        ++count;
                    }
                }
                callback(count);
            });
        });
    }

    /// @brief Minimal workflow start — inserts a RUNNING WorkflowExecution for `def_name` and
    /// delivers its id. Full DAG advancement stays engine-side (the engine never routes
    /// start_workflow through this backend); this is here for interface completeness + external
    /// callers.
    static void start_workflow(
        void* connector_ctx,
        std::string def_name,
        std::unordered_map<std::string, std::string> variables,
        std::move_only_function<void(std::optional<std::string>)> callback
    )
    {
        if (connector_ctx == nullptr) {
            callback(std::nullopt);
            return;
        }
        auto& conn = *static_cast<connector::Connector*>(connector_ctx);
        conn.find<model::WorkflowDef>(
            def_name,
            [&conn, variables = std::move(variables),
             callback = std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                if (!def) {
                    callback(std::nullopt);
                    return;
                }
                model::WorkflowExecution exec;
                exec.set_exec_id(model::generate_id());
                exec.set_def_name(def->get_name());
                exec.set_def_version(def->get_version());
                exec.set_status(model::WorkflowStatus::RUNNING);
                exec.set_variables(std::move(variables));
                auto exec_id = std::format("{}", exec.get_exec_id());
                conn.insert<model::WorkflowExecution>(
                    std::move(exec), [exec_id = std::move(exec_id),
                                      callback = std::move(callback)](bool oke) mutable {
                        callback(
                            oke ? std::optional<std::string>{std::move(exec_id)} : std::nullopt
                        );
                    }
                );
            }
        );
    }
};

} // namespace worker_orchestrator

#ifdef CONGELADO_TEST
namespace worker_orchestrator::store_tests {
using namespace boost::ut;

/// @brief Trivial synchronous in-memory ICache — Connector aborts via active_cache() if none is
/// wired in, so every OrchestratorStore test needs one of these behind its connector_ctx.
class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }

    void set(
        std::string_view key, std::string_view value, shared::QueryReadFn&& result
    ) noexcept override
    {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }

    void remove(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        m_store.erase(std::string{key});
        result("ok");
    }

private:
    std::unordered_map<std::string, std::string> m_store;
};

class StoreFixture
{
public:
    StoreFixture()
    {
        m_connector.set_cache(&m_cache);
    }

    [[nodiscard]] connector::Connector& get_connector() noexcept
    {
        return m_connector;
    }

    [[nodiscard]] void* ctx() noexcept
    {
        return &m_connector;
    }

private:
    FakeCache m_cache;
    connector::Connector m_connector;
};

suite<"OrchestratorStore::enqueue"> store_enqueue_suite = [] {
    "a nullptr connector_ctx reports std::nullopt"_test = [] {
        std::optional<std::string> result{"unset"};
        OrchestratorStore::enqueue(
            nullptr, "echo", serde::Value{serde::Value::Object{}},
            [&result](std::optional<std::string> id) {
                result = id;
            }
        );
        expect(!result.has_value());
    };

    "inserts a SCHEDULED TaskInstance and hands back its id"_test = [] {
        StoreFixture fixture;
        serde::Value::Object input;
        input["to"] = std::string{"a@example.com"};

        std::optional<std::string> result;
        OrchestratorStore::enqueue(
            fixture.ctx(), "echo", serde::Value{std::move(input)},
            [&result](std::optional<std::string> id) {
                result = id;
            }
        );

        expect(result.has_value()) << fatal;
        std::optional<model::TaskInstance> found;
        fixture.get_connector().find<model::TaskInstance>(
            *result, [&found](std::optional<model::TaskInstance> value) {
                found = std::move(value);
            }
        );
        expect(found.has_value()) << fatal;
        expect(found->get_def_name() == "echo");
        expect(found->get_status() == model::TaskStatus::SCHEDULED);
    };
};

suite<"OrchestratorStore::claim"> store_claim_suite = [] {
    "a nullptr connector_ctx reports std::nullopt"_test = [] {
        std::optional<std::string> result{"unset"};
        OrchestratorStore::claim(
            nullptr, "echo", std::nullopt, [&result](std::optional<std::string> value) {
                result = value;
            }
        );
        expect(!result.has_value());
    };

    "an empty queue reports std::nullopt"_test = [] {
        StoreFixture fixture;
        std::optional<std::string> result{"unset"};
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::nullopt, [&result](std::optional<std::string> value) {
                result = value;
            }
        );
        expect(!result.has_value());
    };

    "claims the oldest SCHEDULED instance whose def's worker_type matches, flips it "
    "IN_PROGRESS, returns it serialized"_test = [] {
        StoreFixture fixture;
        model::TaskDef def;
        def.set_name("send_email");
        def.set_type(model::TaskType::SIMPLE);
        def.set_worker_type("echo");
        fixture.get_connector().insert<model::TaskDef>(def, [](bool) {});

        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        std::optional<std::string> result;
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::nullopt, [&result](std::optional<std::string> value) {
                result = value;
            }
        );

        expect(result.has_value()) << fatal;
        expect(!result->empty());

        std::optional<model::TaskInstance> updated;
        fixture.get_connector().find<model::TaskInstance>(
            std::format("{}", instance.get_task_id()),
            [&updated](std::optional<model::TaskInstance> value) {
                updated = std::move(value);
            }
        );
        expect(updated.has_value()) << fatal;
        expect(updated->get_status() == model::TaskStatus::IN_PROGRESS);
        expect(updated->get_deadline_at().has_value());
    };

    "a worker_type that doesn't match any def's worker_type reports std::nullopt"_test = [] {
        StoreFixture fixture;
        model::TaskDef def;
        def.set_name("send_email");
        def.set_worker_type("echo");
        fixture.get_connector().insert<model::TaskDef>(def, [](bool) {});
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        std::optional<std::string> result{"unset"};
        OrchestratorStore::claim(
            fixture.ctx(), "other_worker_type", std::nullopt,
            [&result](std::optional<std::string> value) {
                result = value;
            }
        );
        expect(!result.has_value());
    };

    "a domain filter only matches instances whose def's domain agrees exactly"_test = [] {
        StoreFixture fixture;
        model::TaskDef def;
        def.set_name("send_email");
        def.set_worker_type("echo");
        def.set_domain("tenant_a");
        fixture.get_connector().insert<model::TaskDef>(def, [](bool) {});
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        std::optional<std::string> wrong_domain{"unset"};
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::optional<std::string>{"tenant_b"},
            [&wrong_domain](std::optional<std::string> value) {
                wrong_domain = value;
            }
        );
        expect(!wrong_domain.has_value());

        std::optional<std::string> right_domain;
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::optional<std::string>{"tenant_a"},
            [&right_domain](std::optional<std::string> value) {
                right_domain = value;
            }
        );
        expect(right_domain.has_value());
    };

    "already-claimed (non-SCHEDULED) instances are never re-claimed"_test = [] {
        StoreFixture fixture;
        model::TaskDef def;
        def.set_name("send_email");
        def.set_worker_type("echo");
        fixture.get_connector().insert<model::TaskDef>(def, [](bool) {});
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        std::optional<std::string> first;
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::nullopt, [&first](std::optional<std::string> value) {
                first = value;
            }
        );
        expect(first.has_value()) << fatal;

        std::optional<std::string> second{"unset"};
        OrchestratorStore::claim(
            fixture.ctx(), "echo", std::nullopt, [&second](std::optional<std::string> value) {
                second = value;
            }
        );
        expect(!second.has_value());
    };
};

suite<"OrchestratorStore::submit_result"> store_submit_result_suite = [] {
    "a nullptr connector_ctx reports false"_test = [] {
        bool result = true;
        OrchestratorStore::submit_result(nullptr, "some-id", true, {}, [&result](bool ok) {
            result = ok;
        });
        expect(!result);
    };

    "an unknown task id reports false"_test = [] {
        StoreFixture fixture;
        bool result = true;
        OrchestratorStore::submit_result(
            fixture.ctx(), std::format("{}", model::generate_id()), true, {}, [&result](bool ok) {
                result = ok;
            }
        );
        expect(!result);
    };

    "success=true records COMPLETED with the given output"_test = [] {
        StoreFixture fixture;
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::IN_PROGRESS);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        bool result = false;
        OrchestratorStore::submit_result(
            fixture.ctx(), std::format("{}", instance.get_task_id()), true, {{"message_id", "abc"}},
            [&result](bool ok) {
                result = ok;
            }
        );
        expect(result);

        std::optional<model::TaskInstance> updated;
        fixture.get_connector().find<model::TaskInstance>(
            std::format("{}", instance.get_task_id()),
            [&updated](std::optional<model::TaskInstance> value) {
                updated = std::move(value);
            }
        );
        expect(updated.has_value()) << fatal;
        expect(updated->get_status() == model::TaskStatus::COMPLETED);
        expect(updated->get_output_data().at("message_id") == "abc");
    };

    "success=false records FAILED"_test = [] {
        StoreFixture fixture;
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        instance.set_def_name("send_email");
        instance.set_status(model::TaskStatus::IN_PROGRESS);
        fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

        OrchestratorStore::submit_result(
            fixture.ctx(), std::format("{}", instance.get_task_id()), false, {}, [](bool) {}
        );

        std::optional<model::TaskInstance> updated;
        fixture.get_connector().find<model::TaskInstance>(
            std::format("{}", instance.get_task_id()),
            [&updated](std::optional<model::TaskInstance> value) {
                updated = std::move(value);
            }
        );
        expect(updated.has_value()) << fatal;
        expect(updated->get_status() == model::TaskStatus::FAILED);
    };
};

suite<"OrchestratorStore::queue_size"> store_queue_size_suite = [] {
    "a nullptr connector_ctx reports 0"_test = [] {
        std::size_t result = 99;
        OrchestratorStore::queue_size(nullptr, "echo", [&result](std::size_t count) {
            result = count;
        });
        expect(result == std::size_t{0});
    };

    "counts only SCHEDULED instances whose def's worker_type matches"_test = [] {
        StoreFixture fixture;
        model::TaskDef echo_def;
        echo_def.set_name("echo_task");
        echo_def.set_worker_type("echo");
        fixture.get_connector().insert<model::TaskDef>(echo_def, [](bool) {});
        model::TaskDef other_def;
        other_def.set_name("other_task");
        other_def.set_worker_type("other");
        fixture.get_connector().insert<model::TaskDef>(other_def, [](bool) {});

        model::TaskInstance scheduled_echo;
        scheduled_echo.set_task_id(model::generate_id());
        scheduled_echo.set_def_name("echo_task");
        scheduled_echo.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(scheduled_echo, [](bool) {});

        model::TaskInstance in_progress_echo;
        in_progress_echo.set_task_id(model::generate_id());
        in_progress_echo.set_def_name("echo_task");
        in_progress_echo.set_status(model::TaskStatus::IN_PROGRESS);
        fixture.get_connector().insert<model::TaskInstance>(in_progress_echo, [](bool) {});

        model::TaskInstance scheduled_other;
        scheduled_other.set_task_id(model::generate_id());
        scheduled_other.set_def_name("other_task");
        scheduled_other.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(scheduled_other, [](bool) {});

        std::size_t result = 0;
        OrchestratorStore::queue_size(fixture.ctx(), "echo", [&result](std::size_t count) {
            result = count;
        });
        expect(result == std::size_t{1});
    };
};

suite<"OrchestratorStore::requeue"> store_requeue_suite = [] {
    "a nullptr connector_ctx reports 0"_test = [] {
        std::size_t result = 99;
        OrchestratorStore::requeue(nullptr, "echo", [&result](std::size_t count) {
            result = count;
        });
        expect(result == std::size_t{0});
    };

    "flips IN_PROGRESS instances of the matching worker_type back to SCHEDULED"_test = [] {
        StoreFixture fixture;
        model::TaskDef def;
        def.set_name("echo_task");
        def.set_worker_type("echo");
        fixture.get_connector().insert<model::TaskDef>(def, [](bool) {});

        model::TaskInstance stuck;
        stuck.set_task_id(model::generate_id());
        stuck.set_def_name("echo_task");
        stuck.set_status(model::TaskStatus::IN_PROGRESS);
        fixture.get_connector().insert<model::TaskInstance>(stuck, [](bool) {});

        model::TaskInstance already_scheduled;
        already_scheduled.set_task_id(model::generate_id());
        already_scheduled.set_def_name("echo_task");
        already_scheduled.set_status(model::TaskStatus::SCHEDULED);
        fixture.get_connector().insert<model::TaskInstance>(already_scheduled, [](bool) {});

        std::size_t result = 0;
        OrchestratorStore::requeue(fixture.ctx(), "echo", [&result](std::size_t count) {
            result = count;
        });
        expect(result == std::size_t{1});

        std::optional<model::TaskInstance> updated;
        fixture.get_connector().find<model::TaskInstance>(
            std::format("{}", stuck.get_task_id()),
            [&updated](std::optional<model::TaskInstance> value) {
                updated = std::move(value);
            }
        );
        expect(updated.has_value()) << fatal;
        expect(updated->get_status() == model::TaskStatus::SCHEDULED);
    };
};

suite<"OrchestratorStore::start_workflow"> store_start_workflow_suite = [] {
    "a nullptr connector_ctx reports std::nullopt"_test = [] {
        std::optional<std::string> result{"unset"};
        OrchestratorStore::start_workflow(
            nullptr, "order_pipeline", {}, [&result](std::optional<std::string> id) {
                result = id;
            }
        );
        expect(!result.has_value());
    };

    "a nonexistent def name reports std::nullopt"_test = [] {
        StoreFixture fixture;
        std::optional<std::string> result{"unset"};
        OrchestratorStore::start_workflow(
            fixture.ctx(), "missing_def", {}, [&result](std::optional<std::string> id) {
                result = id;
            }
        );
        expect(!result.has_value());
    };

    "inserts a RUNNING WorkflowExecution with no task instances — DAG advancement stays "
    "engine-side"_test = [] {
        StoreFixture fixture;
        model::WorkflowDef def;
        def.set_name("order_pipeline");
        fixture.get_connector().insert<model::WorkflowDef>(def, [](bool) {});

        std::optional<std::string> result;
        OrchestratorStore::start_workflow(
            fixture.ctx(), "order_pipeline", {{"order_id", "42"}},
            [&result](std::optional<std::string> id) {
                result = id;
            }
        );

        expect(result.has_value()) << fatal;
        std::optional<model::WorkflowExecution> exec;
        fixture.get_connector().find<model::WorkflowExecution>(
            *result, [&exec](std::optional<model::WorkflowExecution> value) {
                exec = std::move(value);
            }
        );
        expect(exec.has_value()) << fatal;
        expect(exec->get_status() == model::WorkflowStatus::RUNNING);
        expect(exec->get_task_instances().empty());
        expect(exec->get_variables().at("order_id") == "42");
    };
};

} // namespace worker_orchestrator::store_tests
#endif

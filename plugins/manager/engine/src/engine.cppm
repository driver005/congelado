export module engine;

export import :context;
export import :task;
export import :workflow;
export import :metadata;
export import :query;
export import :event_handler;
export import :schedule_handler;
export import :admin_handler;
export import :search_handler;
export import :routes;

import std;
import interfaces;
import connector;
import model;
import migration;
import core_logger;
import core_events;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace engine {

/**
 * @brief Wires the SDK-owned shared connector pointer into the engine context.
 *
 * This is defined inside the engine module (rather than in engine.cc) so the plugin
 * implementation unit doesn't have to import the connector module directly — that import
 * triggers a clang modules crash in this translation unit.
 */
inline void set_shared_connector(EngineContext& ctx, void* connector_ctx)
{
    ctx.set_connector(static_cast<connector::Connector*>(connector_ctx));
}

/**
 * @brief Wires the resolved cron backend pointer into the engine context.
 *
 * Defined here (rather than in engine.cc) for the same reason as set_shared_connector — keeps
 * the connector/model imports out of the plugin implementation unit.
 */
inline void set_cron(EngineContext& ctx, void* cron_ctx)
{
    ctx.set_cron(static_cast<interfaces::ICron*>(cron_ctx));
}

/**
 * @brief Installs the engine's cron fire callback and seeds the backend with existing
 * schedules.
 *
 * The cron backend owns timing now; the engine still owns the WorkflowSchedule DB. This
 * callback is what turns a fired job name back into a started workflow — the exact body the old
 * Orchestrator::sweep_schedules ran per due schedule, keyed by one name. Seeding replaces the
 * DB scan the engine no longer does: the cron plugin starts empty, so every currently active
 * schedule is registered here on load. No-op if no cron backend was resolved.
 */
inline void install_cron_scheduling(EngineContext& ctx, interfaces::IWorkflowOrchestrator* workflow)
{
    auto* cron = ctx.get_cron();
    if (cron == nullptr) {
        return;
    }
    cron->set_fire_callback([&ctx, workflow](std::string_view job_name) {
        auto now = std::chrono::system_clock::now();
        ctx.get_connector().find<model::WorkflowSchedule>(
            std::string{job_name},
            [&ctx, workflow, now](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule || !schedule->get_enabled() || schedule->get_paused()) {
                    return;
                }
                schedule->set_last_fired_at(now);
                ctx.get_connector().update<model::WorkflowSchedule>(*schedule, [](bool) {});
                core::logger::info(
                    "engine", "schedule '{}' firing workflow '{}'", schedule->get_name(),
                    schedule->get_workflow_name()
                );
                core::events::publish(
                    "engine.schedule.fired", {{"schedule_name", schedule->get_name()},
                                              {"workflow_name", schedule->get_workflow_name()}}
                );
                if (workflow != nullptr) {
                    workflow->start_workflow(
                        schedule->get_workflow_name(), schedule->get_seed_variables(),
                        [](std::optional<std::string>) {}
                    );
                }
            }
        );
    });
    ctx.get_connector().find_all<model::WorkflowSchedule>(
        [cron](std::vector<model::WorkflowSchedule> schedules) {
            for (auto& schedule: schedules) {
                if (!schedule.get_enabled() || schedule.get_paused()) {
                    continue;
                }
                cron->upsert_job(schedule.get_name(), schedule.get_cron_expression());
            }
        }
    );
}

namespace detail {

    /**
     * @brief Sequentially calls connector.create_table<T>() for every T in Tuple.
     * @tparam I current index in the tuple.
     * @tparam Tuple the tuple of model types.
     * @param conn the connector to create tables through.
     * @param cont continuation called with the final success flag.
     */
    template<std::size_t I, typename Tuple>
    void create_tables(connector::Connector& conn, std::function<void(bool)> cont)
    {
        if constexpr (I >= std::tuple_size_v<Tuple>) {
            cont(true);
        } else {
            using T = std::tuple_element_t<I, Tuple>;
            conn.create_table<T>([&conn, cont = std::move(cont)](bool ok) mutable {
                if (!ok) {
                    core::logger::error(
                        "engine.migrations", "create_table failed for model index {}", I
                    );
                    cont(false);
                    return;
                }
                create_tables<I + 1, Tuple>(conn, std::move(cont));
            });
        }
    }

    void register_engine_baseline()
    {
        migration::Registry::instance().add_baseline(
            "engine", [](interfaces::IDatabase& /*db*/, connector::Connector& conn,
                         std::move_only_function<void(bool)> done) mutable {
                using Tables = model::AllModels;

                auto done_ptr =
                    std::make_shared<std::move_only_function<void(bool)>>(std::move(done));
                std::function<void(bool)> cont = [done_ptr](bool ok) mutable {
                    (*done_ptr)(ok);
                };

                create_tables<0, Tables>(conn, std::move(cont));
            }
        );
    }

} // namespace detail

/**
 * @brief Registers the engine plugin's baseline migration with the shared registry.
 *
 * Called during EnginePlugin::on_load, before the host runs the one global migration pass (see
 * `congelado::heart::App::load_plugins`, after every plugin has loaded and gone ready).
 * Registration only, no run — migrations are a host-owned, cross-plugin concern, not something
 * any one plugin runs on its own; running them per-plugin would mean a plugin loaded after
 * `engine` never got its own baseline picked up.
 */
inline void register_migrations()
{
    detail::register_engine_baseline();
}

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::engine_module_tests {
using namespace boost::ut;

// Minimal fakes — enough to instantiate a real, distinguishable pointer of each interface these
// free functions cast into, and (for FakeCron/FakeWorkflowOrchestrator) to observe what
// install_cron_scheduling() actually does with them.

class FakeDatabase final : public interfaces::IDatabase
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_db";
    }

    void query(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void insert(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void update(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void remove(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }
};

class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void set(
        std::string_view, std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void remove(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }
};

/// @brief Captures the fire callback install_cron_scheduling() registers and every job
/// upsert_job() records, so tests can both inspect seeding and manually trigger a fire.
class FakeCron final : public interfaces::ICron
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cron";
    }

    [[nodiscard]] bool validate(std::string_view) const noexcept override
    {
        return true;
    }

    [[nodiscard]] std::optional<std::chrono::system_clock::time_point>
    next_after(std::string_view, std::chrono::system_clock::time_point) const noexcept override
    {
        return std::nullopt;
    }

    void set_fire_callback(std::move_only_function<void(std::string_view)> callback) override
    {
        m_fire_callback = std::move(callback);
    }

    void upsert_job(std::string_view name, std::string_view cron_expression) override
    {
        m_upserted.emplace_back(std::string{name}, std::string{cron_expression});
    }

    void remove_job(std::string_view) override {}

    /// @brief Invokes the installed fire callback, if one was registered — simulates the cron
    /// backend firing a due job.
    void fire(std::string_view job_name)
    {
        if (m_fire_callback) {
            m_fire_callback(job_name);
        }
    }

    [[nodiscard]] bool has_fire_callback() const noexcept
    {
        return static_cast<bool>(m_fire_callback);
    }

    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>&
    get_upserted() const noexcept
    {
        return m_upserted;
    }

private:
    std::move_only_function<void(std::string_view)> m_fire_callback;
    std::vector<std::pair<std::string, std::string>> m_upserted;
};

class FakeWorkflowOrchestrator final : public interfaces::IWorkflowOrchestrator
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_wf_orch";
    }

    void start_workflow(
        std::string_view workflow_name,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        ++m_start_workflow_count;
        m_last_workflow_name = std::string{workflow_name};
        callback(std::nullopt);
    }

    void on_task_terminal(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void on_execution_terminal(
        std::string_view, std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void pause(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void resume(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void retry(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void restart(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void terminate(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void reconcile(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void rerun(
        std::string_view,
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void signal(
        std::string_view,
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void complete_task(
        std::string_view,
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void start_server() override {}

    void shutdown_all() override {}

    [[nodiscard]] int get_start_workflow_count() const noexcept
    {
        return m_start_workflow_count;
    }

    [[nodiscard]] const std::string& get_last_workflow_name() const noexcept
    {
        return m_last_workflow_name;
    }

private:
    int m_start_workflow_count{0};
    std::string m_last_workflow_name;
};

suite<"set_shared_connector / set_cron"> engine_ctx_wiring_suite = [] {
    "set_shared_connector points the context at the given Connector"_test = [] {
        EngineContext ctx;
        connector::Connector external;
        set_shared_connector(ctx, static_cast<void*>(&external));
        expect(&ctx.get_connector() == &external);
    };

    "set_cron points the context at the given ICron"_test = [] {
        EngineContext ctx;
        expect(ctx.get_cron() == nullptr);
        FakeCron cron;
        set_cron(ctx, static_cast<void*>(&cron));
        expect(ctx.get_cron() == &cron);
    };
};

suite<"install_cron_scheduling"> install_cron_scheduling_suite = [] {
    "no-ops when no cron backend is resolved"_test = [] {
        EngineContext ctx;
        FakeWorkflowOrchestrator workflow;
        expect(nothrow([&] {
            install_cron_scheduling(ctx, &workflow);
        }));
        expect(workflow.get_start_workflow_count() == 0);
    };

    "installs a fire callback and seeds only enabled, unpaused schedules into the cron backend"_test =
        [] {
            EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            FakeCron cron;
            ctx.set_cron(&cron);

            model::WorkflowSchedule enabled;
            enabled.set_name("nightly");
            enabled.set_workflow_name("report_pipeline");
            enabled.set_cron_expression("0 0 * * *");
            bool seeded_enabled = false;
            ctx.get_connector().upsert<model::WorkflowSchedule>(
                enabled, [&seeded_enabled](bool ok) {
                    seeded_enabled = ok;
                }
            );
            expect(seeded_enabled) << fatal;

            model::WorkflowSchedule disabled;
            disabled.set_name("disabled_one");
            disabled.set_workflow_name("x");
            disabled.set_cron_expression("* * * * *");
            disabled.set_enabled(false);
            bool seeded_disabled = false;
            ctx.get_connector().upsert<model::WorkflowSchedule>(
                disabled, [&seeded_disabled](bool ok) {
                    seeded_disabled = ok;
                }
            );
            expect(seeded_disabled) << fatal;

            model::WorkflowSchedule paused;
            paused.set_name("paused_one");
            paused.set_workflow_name("y");
            paused.set_cron_expression("* * * * *");
            paused.set_paused(true);
            bool seeded_paused = false;
            ctx.get_connector().upsert<model::WorkflowSchedule>(paused, [&seeded_paused](bool ok) {
                seeded_paused = ok;
            });
            expect(seeded_paused) << fatal;

            FakeWorkflowOrchestrator workflow;
            install_cron_scheduling(ctx, &workflow);

            expect(cron.has_fire_callback());
            auto& upserted = cron.get_upserted();
            expect(upserted.size() == 1U) << fatal;
            expect(upserted[0].first == "nightly");
            expect(upserted[0].second == "0 0 * * *");
        };

    "the installed fire callback no-ops for a job name with no matching schedule"_test = [] {
        EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        FakeCron cron;
        ctx.set_cron(&cron);
        FakeWorkflowOrchestrator workflow;
        install_cron_scheduling(ctx, &workflow);
        expect(cron.has_fire_callback()) << fatal;

        expect(nothrow([&] {
            cron.fire("does-not-exist");
        }));
        expect(workflow.get_start_workflow_count() == 0);
    };

    "the installed fire callback starts the workflow for an enabled, unpaused schedule"_test = [] {
        EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        FakeCron cron;
        ctx.set_cron(&cron);

        model::WorkflowSchedule schedule;
        schedule.set_name("nightly");
        schedule.set_workflow_name("report_pipeline");
        schedule.set_cron_expression("0 0 * * *");
        bool seeded = false;
        ctx.get_connector().upsert<model::WorkflowSchedule>(schedule, [&seeded](bool ok) {
            seeded = ok;
        });
        expect(seeded) << fatal;

        FakeWorkflowOrchestrator workflow;
        install_cron_scheduling(ctx, &workflow);
        expect(cron.has_fire_callback()) << fatal;

        cron.fire("nightly");
        expect(workflow.get_start_workflow_count() == 1);
        expect(workflow.get_last_workflow_name() == "report_pipeline");
    };

    "the installed fire callback skips a found-but-disabled schedule"_test = [] {
        EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        FakeCron cron;
        ctx.set_cron(&cron);

        model::WorkflowSchedule schedule;
        schedule.set_name("nightly");
        schedule.set_workflow_name("report_pipeline");
        schedule.set_cron_expression("0 0 * * *");
        schedule.set_enabled(false);
        bool seeded = false;
        ctx.get_connector().upsert<model::WorkflowSchedule>(schedule, [&seeded](bool ok) {
            seeded = ok;
        });
        expect(seeded) << fatal;

        FakeWorkflowOrchestrator workflow;
        install_cron_scheduling(ctx, &workflow);
        cron.fire("nightly");
        expect(workflow.get_start_workflow_count() == 0);
    };

    "the installed fire callback tolerates a null workflow backend instead of crashing"_test = [] {
        EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        FakeCron cron;
        ctx.set_cron(&cron);

        model::WorkflowSchedule schedule;
        schedule.set_name("nightly");
        schedule.set_workflow_name("report_pipeline");
        schedule.set_cron_expression("0 0 * * *");
        bool seeded = false;
        ctx.get_connector().upsert<model::WorkflowSchedule>(schedule, [&seeded](bool ok) {
            seeded = ok;
        });
        expect(seeded) << fatal;

        install_cron_scheduling(ctx, nullptr);
        expect(nothrow([&] {
            cron.fire("nightly");
        }));
    };
};

suite<"register_migrations"> register_migrations_suite = [] {
    "registers exactly one \"engine\" baseline that create_table()s every model and reports success"_test =
        [] {
            auto& registry = migration::Registry::instance();
            auto before = registry.baselines().size();

            register_migrations();

            expect(registry.baselines().size() == before + 1) << fatal;
            auto& [name, fn] = registry.baselines().back();
            expect(name == "engine");

            // Unused by the registered lambda body (it ignores its IDatabase& param), but the
            // callback signature still needs a live reference to bind.
            FakeDatabase db;
            // No database configured — create_table<T>() is a synchronous local-store no-op
            // success for every T in model::AllModels.
            connector::Connector conn;
            bool done_called = false;
            bool done_ok = false;
            fn(db, conn, [&done_called, &done_ok](bool ok) {
                done_called = true;
                done_ok = ok;
            });

            expect(done_called);
            expect(done_ok);
        };
};

} // namespace engine::engine_module_tests
#endif

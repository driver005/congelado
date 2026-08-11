export module engine;

export import :context;
export import :expr;
export import :system_task;
export import :schema;
export import :search_projector;
export import :local_payload_storage;
export import :orchestrator;
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

export namespace engine {

/**
 * @brief Wires the SDK-owned shared connector pointer into the engine context.
 *
 * This is defined inside the engine module (rather than in engine.cc) so the plugin
 * implementation unit doesn't have to import the connector module directly — that import
 * triggers a clang modules crash in this translation unit.
 */
inline void set_shared_connector(EngineContext &ctx, void *connector_ctx) {
    ctx.set_connector(static_cast<connector::Connector *>(connector_ctx));
}

/**
 * @brief Wires the resolved cron backend pointer into the engine context.
 *
 * Defined here (rather than in engine.cc) for the same reason as set_shared_connector — keeps the
 * connector/model imports out of the plugin implementation unit.
 */
inline void set_cron(EngineContext &ctx, void *cron_ctx) {
    ctx.set_cron(static_cast<interfaces::ICron *>(cron_ctx));
}

/**
 * @brief Installs the engine's cron fire callback and seeds the backend with existing schedules.
 *
 * The cron backend owns timing now; the engine still owns the WorkflowSchedule DB. This callback
 * is what turns a fired job name back into a started workflow — the exact body the old
 * Orchestrator::sweep_schedules ran per due schedule, keyed by one name. Seeding replaces the
 * DB scan the engine no longer does: the cron plugin starts empty, so every currently active
 * schedule is registered here on load. No-op if no cron backend was resolved.
 */
inline void install_cron_scheduling(EngineContext &ctx, Orchestrator &orchestrator) {
    auto *cron = ctx.get_cron();
    if (cron == nullptr) {
        return;
    }
    cron->set_fire_callback([&ctx, &orchestrator](std::string_view job_name) {
        auto now = std::chrono::system_clock::now();
        ctx.get_connector().find<model::WorkflowSchedule>(
            std::string{job_name},
            [&ctx, &orchestrator, now](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule || !schedule->get_enabled() || schedule->get_paused()) {
                    return;
                }
                schedule->set_last_fired_at(now);
                ctx.get_connector().update<model::WorkflowSchedule>(*schedule,
                                                                    [](bool) {});
                core::logger::info("engine", "schedule '{}' firing workflow '{}'",
                                   schedule->get_name(), schedule->get_workflow_name());
                core::events::publish("engine.schedule.fired",
                                      {{"schedule_name", schedule->get_name()},
                                       {"workflow_name", schedule->get_workflow_name()}});
                orchestrator.start(schedule->get_workflow_name(), schedule->get_seed_variables(),
                                   std::nullopt, [](std::optional<model::WorkflowExecution>) {});
            });
    });
    ctx.get_connector().find_all<model::WorkflowSchedule>(
        [cron](std::vector<model::WorkflowSchedule> schedules) {
            for (auto &schedule : schedules) {
                if (!schedule.get_enabled() || schedule.get_paused()) {
                    continue;
                }
                cron->upsert_job(schedule.get_name(), schedule.get_cron_expression());
            }
        });
}

namespace detail {

/**
 * @brief Sequentially calls connector.create_table<T>() for every T in Tuple.
 * @tparam I current index in the tuple.
 * @tparam Tuple the tuple of model types.
 * @param conn the connector to create tables through.
 * @param cont continuation called with the final success flag.
 */
template <std::size_t I, typename Tuple>
void create_tables(connector::Connector &conn, std::function<void(bool)> cont) {
    if constexpr (I >= std::tuple_size_v<Tuple>) {
        cont(true);
    } else {
        using T = std::tuple_element_t<I, Tuple>;
        conn.create_table<T>([&conn, cont = std::move(cont)](bool ok) mutable {
            if (!ok) {
                core::logger::error("engine.migrations",
                                    "create_table failed for model index {}", I);
                cont(false);
                return;
            }
            create_tables<I + 1, Tuple>(conn, std::move(cont));
        });
    }
}

void register_engine_baseline() {
    migration::Registry::instance().add_baseline(
        "engine",
        [](interfaces::IDatabase & /*db*/, connector::Connector &conn,
           std::move_only_function<void(bool)> done) mutable {
            using Tables = model::AllModels;

            auto done_ptr =
                std::make_shared<std::move_only_function<void(bool)>>(std::move(done));
            std::function<void(bool)> cont =
                [done_ptr](bool ok) mutable { (*done_ptr)(ok); };

            create_tables<0, Tables>(conn, std::move(cont));
        });
}

} // namespace detail

/**
 * @brief Registers the engine plugin's baseline migration with the shared registry.
 *
 * Called during EnginePlugin::on_load, before the host runs the one global migration pass (see
 * `congelado::heart::App::load_plugins`, after every plugin has loaded and gone ready). Registration
 * only, no run — migrations are a host-owned, cross-plugin concern, not something any one plugin
 * runs on its own; running them per-plugin would mean a plugin loaded after `engine` never got
 * its own baseline picked up.
 */
inline void register_migrations() { detail::register_engine_baseline(); }

} // namespace engine

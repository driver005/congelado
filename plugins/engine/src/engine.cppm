export module engine;

export import :context;
export import :expr;
export import :system_task;
export import :cron;
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

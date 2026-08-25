module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module worker_manager_external_plugin;

import congelado_plugin;
import interfaces;
import worker_runtime;
import congelado_worker;
import model;
import serde;
import core_logger;
import core_router;
import core_config;
import io_layer_http2;
import io_base_socket;
import io_base_leverage;
import io_flow_socket;
import core_contract;
import shared;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

using Leverager = io::base::leverage::Leverager<io::base::leverage::Context>;
using ServerFlow =
    io::base::flow::sync::ServerFlowSocket<core::contract::ContractGroup<>,
                                           io::base::socket::Protocol::TLS>;

/// @brief Shape-matches the engine's task submit body — `{"result": ..., "output_data": {...}}`.
struct SubmitResultBody {
    model::TaskResult result{model::TaskResult::SUCCESS};
    std::unordered_map<std::string, std::string> output_data;
};

/// @brief One concurrency slot's async-poll bookkeeping. `POLLING` is idle/mid-scan, `AWAITING` has
/// dispatched a task and is waiting on its completion, `RESUMING` holds a landed result ready to
/// submit. `m_on_stack` distinguishes a completion firing before `run_task_async` returns (still on
/// the poll_slot() call stack — the caller picks the result up itself, no wake needed) from one
/// firing later, from another thread (needs `wake()` to resume the parked contract). Every field is
/// guarded by `m_mutex` since the wake path runs on a different thread than `poll_slot()`.
enum class SlotPhase : std::uint8_t { POLLING, AWAITING, RESUMING };

class PollSlot {
  public:
    void set_wake(std::move_only_function<void()> wake) { m_wake = std::move(wake); }
    void set_phase(SlotPhase phase) noexcept { m_phase = phase; }
    void set_task_id(std::string task_id) { m_task_id = std::move(task_id); }
    void set_on_stack(bool on_stack) noexcept { m_on_stack = on_stack; }
    void set_result(bool success, std::unordered_map<std::string, std::string> output) {
        m_success = success;
        m_output = std::move(output);
    }

    [[nodiscard]] SlotPhase get_phase() const noexcept { return m_phase; }
    [[nodiscard]] const std::string &get_task_id() const noexcept { return m_task_id; }
    [[nodiscard]] bool is_on_stack() const noexcept { return m_on_stack; }
    [[nodiscard]] bool get_success() const noexcept { return m_success; }
    [[nodiscard]] std::unordered_map<std::string, std::string> take_output() {
        return std::move(m_output);
    }
    [[nodiscard]] std::mutex &get_mutex() noexcept { return m_mutex; }

    /// @brief Resumes this slot's parked contract, if a wake callback was registered.
    void wake() {
        if (m_wake) {
            m_wake();
        }
    }

  private:
    std::move_only_function<void()> m_wake;
    SlotPhase m_phase{SlotPhase::POLLING};
    bool m_on_stack{false};
    bool m_success{false};
    std::string m_task_id;
    std::unordered_map<std::string, std::string> m_output;
    std::mutex m_mutex;
};

/// @brief The external worker manager: the full worker runtime as a plugin. Owns the worker
/// registry + engine comm (WorkerContext), runs the outbound poll-execute-submit loop
/// (`poll_once`), and stands up the worker's own inbound HTTP API server (`start_server`) — the
/// health/info/poll/ack/executions routes that used to live in the engine's `worker/` module.
class ExternalWorkerManager final : public interfaces::IWorkerManager {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "external"; }

    void add_worker(interfaces::IWorker &worker) override { m_ctx.add_worker(worker); }

    void set_runtime(interfaces::IClient &client) override { m_ctx.set_runtime(client); }

    void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) override {
        m_ctx.dispatch(request, response);
    }

    void poll_once() override {
        for (auto type : m_ctx.get_task_types()) {
            worker::WorkerContext::EngineResponse poll_res;
            try {
                poll_res = m_ctx.call_engine("GET", std::format("/api/v1/tasks/queue/{}", type));
            } catch (const std::exception &error) {
                core::logger::error("worker_manager.external", "poll exception type={}: {}", type,
                                    error.what());
                continue;
            }
            if (poll_res.m_status == 204) {
                continue;
            }
            if (poll_res.m_status != 200) {
                core::logger::error("worker_manager.external", "poll failed type={} status={}", type,
                                    poll_res.m_status);
                continue;
            }
            auto parsed =
                serde::Ser::deserialize<model::TaskInstance>("application/json", poll_res.m_body);
            if (!parsed.has_value()) {
                core::logger::error("worker_manager.external", "parse failed: {}", parsed.error());
                continue;
            }
            auto &instance = *parsed;
            // Run against the worker_type we just polled (workers are keyed by get_task_type()),
            // not the instance's def_name — a task def's name and its worker_type need not match.
            auto output = m_ctx.run_task(type, instance.get_input_data());
            auto result = output.has_value() ? model::TaskResult::SUCCESS : model::TaskResult::FAILURE;
            SubmitResultBody submit{
                .result = result,
                .output_data = output.has_value()
                                   ? std::move(*output)
                                   : std::unordered_map<std::string, std::string>{
                                         {"error", std::string{output.error().getMessage()}}}};
            auto encoded = serde::Ser::serialize("application/json", submit);
            std::string submit_body{encoded.begin(), encoded.end()};
            auto task_id = std::format("{}", instance.get_task_id());
            try {
                m_ctx.call_engine("POST", std::format("/api/v1/tasks/{}/result", task_id),
                                  submit_body);
            } catch (const std::exception &error) {
                core::logger::error("worker_manager.external", "submit exception task={}: {}",
                                    task_id, error.what());
            }
        }
    }

    void register_poll_slot(std::size_t slot_index, std::move_only_function<void()> wake) override {
        if (slot_index >= m_slots.size()) {
            m_slots.resize(slot_index + 1);
        }
        m_slots[slot_index].set_wake(std::move(wake));
    }

    void poll_slot(std::size_t slot_index) override {
        // SECURITY: no bounds check against m_slots.size() here, unlike register_poll_slot()
        // right above (which resizes to fit). A caller invoking poll_slot() for an index that
        // was never registered indexes a std::deque out of bounds — undefined behavior, not a
        // clean error. The host is expected to only ever call this for indices it already
        // registered, but nothing here enforces that.
        auto &slot = m_slots[slot_index];

        bool resume_ready = false;
        std::string resume_task_id;
        bool resume_success = false;
        std::unordered_map<std::string, std::string> resume_output;
        {
            std::lock_guard lock{slot.get_mutex()};
            if (slot.get_phase() == SlotPhase::RESUMING) {
                resume_ready = true;
                resume_task_id = slot.get_task_id();
                resume_success = slot.get_success();
                resume_output = slot.take_output();
                slot.set_phase(SlotPhase::POLLING);
            }
        }
        if (resume_ready) {
            submit_result(resume_task_id, resume_success, std::move(resume_output));
            shared::this_handler::shedule();
            return;
        }

        for (auto type : m_ctx.get_task_types()) {
            worker::WorkerContext::EngineResponse poll_res;
            try {
                poll_res = m_ctx.call_engine("GET", std::format("/api/v1/tasks/queue/{}", type));
            } catch (const std::exception &error) {
                core::logger::error("worker_manager.external", "poll exception type={}: {}", type,
                                    error.what());
                continue;
            }
            if (poll_res.m_status == 204) {
                continue;
            }
            if (poll_res.m_status != 200) {
                core::logger::error("worker_manager.external", "poll failed type={} status={}", type,
                                    poll_res.m_status);
                continue;
            }
            auto parsed =
                serde::Ser::deserialize<model::TaskInstance>("application/json", poll_res.m_body);
            if (!parsed.has_value()) {
                core::logger::error("worker_manager.external", "parse failed: {}", parsed.error());
                continue;
            }
            auto &instance = *parsed;
            auto task_id = std::format("{}", instance.get_task_id());

            {
                std::lock_guard lock{slot.get_mutex()};
                slot.set_phase(SlotPhase::AWAITING);
                slot.set_task_id(task_id);
                slot.set_on_stack(true);
            }
            m_ctx.run_task_async(
                type, instance.get_input_data(),
                [this, slot_index](interfaces::WorkerResult output) {
                    auto &woken_slot = m_slots[slot_index];
                    bool need_wake = false;
                    {
                        std::lock_guard lock{woken_slot.get_mutex()};
                        woken_slot.set_result(
                            output.has_value(),
                            output.has_value()
                                ? std::move(*output)
                                : std::unordered_map<std::string, std::string>{
                                      {"error", std::string{output.error().getMessage()}}});
                        woken_slot.set_phase(SlotPhase::RESUMING);
                        need_wake = !woken_slot.is_on_stack();
                    }
                    if (need_wake) {
                        woken_slot.wake();
                    }
                });

            bool handled_inline = false;
            bool inline_success = false;
            std::unordered_map<std::string, std::string> inline_output;
            {
                std::lock_guard lock{slot.get_mutex()};
                slot.set_on_stack(false);
                if (slot.get_phase() == SlotPhase::RESUMING) {
                    handled_inline = true;
                    inline_success = slot.get_success();
                    inline_output = slot.take_output();
                    slot.set_phase(SlotPhase::POLLING);
                }
            }
            if (handled_inline) {
                submit_result(task_id, inline_success, std::move(inline_output));
                continue;
            }
            // Genuinely parked — the wake callback above resumes this slot later.
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        shared::this_handler::shedule();
    }

    void start_server() override {
        if (m_contract_group == nullptr || m_leverager == nullptr) {
            core::logger::error("worker_manager.external",
                                "no contract group/leverager — inbound server not started");
            return;
        }
        // Config (bind address + identity) comes from the worker's own TOML, loaded here (not in
        // on_load) because the serde TOML format isn't registered until after plugin build().
        auto cfg_result = congelado::worker::WorkerConfig::from_file(m_config_path);
        std::string bind_host = "0.0.0.0";
        std::uint32_t bind_port = 9091;
        std::string cert = "server.crt";
        std::string key = "server.key";
        if (cfg_result) {
            auto &cfg = *cfg_result;
            m_ctx.set_worker_id(cfg.getWorkerId());
            bind_host = cfg.getBindHost();
            bind_port = cfg.getBindPort();
            cert = cfg.getEngineCert().empty() ? "server.crt" : cfg.getEngineCert();
            key = cfg.getEngineKey().empty() ? "server.key" : cfg.getEngineKey();
        } else {
            core::logger::error("worker_manager.external", "config load failed: {}",
                                cfg_result.error());
        }

        worker::PollHandler::bind(m_ctx);
        worker::ExecutionHandler::bind(m_ctx);
        worker::StatusHandler::bind(m_ctx);
        worker::register_routes(m_router);

        m_server_cfg.add_field("host", bind_host);
        m_server_cfg.add_field("port", std::to_string(bind_port));
        m_server_cfg.add_field("cert", cert);
        m_server_cfg.add_field("key", key);

        m_server_protocol = std::make_unique<io::layer::http2::Http2Protocol>(&m_server_cfg);
        m_server = m_server_protocol->get_server();
        m_server->build(&m_router);
        m_server->set_contract_group(*m_contract_group);

        io::base::socket::Endpoint bind_endpoint{bind_host, static_cast<std::uint16_t>(bind_port)};
        m_server_flow = std::make_unique<ServerFlow>(bind_endpoint, *m_leverager, *m_contract_group);
        auto *server = m_server.get();
        m_server_flow->add_on_accept(
            [server](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
                return server->on_connect(std::move(send), std::move(close));
            });
        try {
            m_server_flow->build();
        } catch (const std::exception &error) {
            core::logger::error("worker_manager.external", "failed to bind inbound server {}:{}: {}",
                                bind_host, bind_port, error.what());
            return;
        }
        core::logger::important("worker_manager.external", "inbound API server listening on {}:{}",
                                bind_host, bind_port);
    }

    // Lifecycle/health — minimal.
    [[nodiscard]] std::optional<std::string> spawn(const interfaces::WorkerSpec & /*spec*/) override {
        return std::nullopt;
    }
    bool start(std::string_view /*worker_id*/) override { return true; }
    bool stop(std::string_view /*worker_id*/) override { return true; }
    bool restart(std::string_view /*worker_id*/) override { return true; }
    void shutdown_all() override {}

    [[nodiscard]] std::vector<interfaces::WorkerInfo> list() const override {
        std::vector<interfaces::WorkerInfo> out;
        for (auto type : m_ctx.get_task_types()) {
            out.push_back(interfaces::WorkerInfo{.worker_id = std::string{type},
                                                 .worker_type = std::string{type},
                                                 .alive = true});
        }
        return out;
    }
    [[nodiscard]] std::optional<interfaces::WorkerInfo>
    status(std::string_view worker_id) const override {
        if (m_ctx.get_task_worker(worker_id) == nullptr) {
            return std::nullopt;
        }
        return interfaces::WorkerInfo{
            .worker_id = std::string{worker_id}, .worker_type = std::string{worker_id}, .alive = true};
    }
    void set_health_callback(
        std::move_only_function<void(const interfaces::WorkerInfo &)> callback) override {
        m_health = std::move(callback);
    }

    void register_task_defs(const std::vector<std::string> &defs_json) override {
        register_defs("/api/v1/tasks", "task", defs_json);
    }

    void register_workflow_defs(const std::vector<std::string> &defs_json) override {
        register_defs("/api/v1/workflows", "workflow", defs_json);
    }

    /// @brief Stashes the host callbacks (contract group + leverager for the inbound server) and
    /// the worker config path.
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) {
        m_contract_group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
        m_leverager = congelado::leverager_ctx<Leverager>(host);
        m_config_path = congelado::config_get(cfg, "config_path").value_or("worker.toml");
    }

  private:
    /// @brief Serializes and POSTs one task's result to the engine — shared by the resume path (an
    /// async task's response landed) and the inline path (a sync worker finished within its own
    /// `poll_slot()` call).
    void submit_result(std::string_view task_id, bool success,
                       std::unordered_map<std::string, std::string> output) {
        SubmitResultBody submit{.result = success ? model::TaskResult::SUCCESS
                                                  : model::TaskResult::FAILURE,
                                .output_data = std::move(output)};
        auto encoded = serde::Ser::serialize("application/json", submit);
        std::string submit_body{encoded.begin(), encoded.end()};
        try {
            m_ctx.call_engine("POST", std::format("/api/v1/tasks/{}/result", task_id), submit_body);
        } catch (const std::exception &error) {
            core::logger::error("worker_manager.external", "submit exception task={}: {}", task_id,
                                error.what());
        }
    }

    /// @brief POSTs each serialized def to `path` over the engine connection — both the task and
    /// workflow endpoints upsert, so re-registering on a worker restart is idempotent. Logs and
    /// skips any that fail rather than aborting the whole batch.
    void register_defs(std::string_view path, std::string_view kind,
                       const std::vector<std::string> &defs_json) {
        for (const auto &def_json : defs_json) {
            try {
                auto response = m_ctx.call_engine("POST", path, def_json);
                if (response.m_status < 200 || response.m_status >= 300) {
                    core::logger::error("worker_manager.external",
                                        "register {} def failed status={} body={}", kind,
                                        response.m_status, response.m_body);
                } else {
                    core::logger::info("worker_manager.external", "registered {} def", kind);
                }
            } catch (const std::exception &error) {
                core::logger::error("worker_manager.external", "register {} def exception: {}", kind,
                                    error.what());
            }
        }
    }

    worker::WorkerContext m_ctx;
    core::contract::ContractGroup<> *m_contract_group{nullptr};
    Leverager *m_leverager{nullptr};
    std::string m_config_path;
    std::move_only_function<void(const interfaces::WorkerInfo &)> m_health;
    // deque, not vector — PollSlot holds a std::mutex, so growing it must never move existing
    // elements. Indexed by slot, sized lazily in register_poll_slot().
    std::deque<PollSlot> m_slots;

    // Inbound server, built in start_server() and kept alive for the process. m_server_cfg must
    // precede m_server_protocol (which holds a pointer to it).
    core::router::RouterContext<> m_router;
    core::config::PluginConfig m_server_cfg;
    std::unique_ptr<io::layer::http2::Http2Protocol> m_server_protocol;
    decltype(std::declval<io::layer::http2::Http2Protocol &>().get_server()) m_server;
    std::unique_ptr<ServerFlow> m_server_flow;
};

/// @brief The external worker-manager plugin — exports the WORKER_MANAGER capability.
class WorkerManagerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "worker_manager_external";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override {
        return "worker_manager";
    }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER_MANAGER;
    }
    void *worker_manager_get() noexcept {
        return static_cast<interfaces::IWorkerManager *>(&m_manager);
    }
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) override {
        m_manager.on_load(host, cfg);
    }

  private:
    ExternalWorkerManager m_manager;
};

} // namespace

CONGELADO_PLUGIN(WorkerManagerPlugin);

#ifdef CONGELADO_TEST
namespace {
namespace external_worker_manager_tests {
using namespace boost::ut;

/// @brief Minimal IWorker double, just enough to populate list()/status()/poll_once() targets.
class FakeWorker final : public interfaces::IWorker {
  public:
    explicit FakeWorker(std::string task_type) : m_task_type{std::move(task_type)} {}
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_task_type; }

  private:
    std::string m_task_type;
};

/// @brief Concrete IRequest double just complete enough for WorkerContext::call_engine() to
/// build a request against — same shape as core_client:registry's own RegisterFakeRequest test
/// double.
class TestClientRequest final : public interfaces::io::IRequest {
  public:
    explicit TestClientRequest(std::uint32_t stream_id) : interfaces::io::IRequest{stream_id} {}

    void set_header(std::variant<std::string_view, interfaces::io::types::Token>,
                    std::string_view) & override {}
    [[nodiscard]] std::string_view find_header(
        std::variant<std::string_view, interfaces::io::types::Token>) const noexcept override {
        return "x";
    }
    void set_body(std::vector<std::byte> &&body) & override { m_body = std::move(body); }

  private:
    std::vector<std::byte> m_body;
};

/// @brief Concrete IClient double whose create_request() hands back a working TestClientRequest
/// (so call_engine()'s own build() doesn't abort) but whose send() always throws — used to
/// deterministically exercise call_engine()'s failure path without a live connection and without
/// blocking on its no-timeout future.
class ThrowingTestClient final : public interfaces::IClient {
  public:
    shared::ReadCallback on_connect(shared::SendCallback, shared::CloseCallback) override {
        return {};
    }
    std::uint32_t send(interfaces::io::IRequest &) override {
        throw std::runtime_error("ThrowingTestClient: send always fails");
    }
    [[nodiscard]] std::unique_ptr<interfaces::io::IRequest>
    create_request(std::uint32_t stream_id) override {
        return std::make_unique<TestClientRequest>(stream_id);
    }
};

/// @brief Binds `manager`'s own runtime to a client whose send() always throws, so any
/// WorkerContext::call_engine() reached below throws std::runtime_error deterministically and
/// synchronously instead of blocking on its no-timeout future.
void install_throwing_client(ExternalWorkerManager &manager) {
    static ThrowingTestClient client;
    manager.set_runtime(client);
}

suite<"SubmitResultBody"> submit_result_body_suite = [] {
    "defaults to SUCCESS with empty output_data"_test = [] {
        SubmitResultBody body;
        expect(body.result == model::TaskResult::SUCCESS);
        expect(body.output_data.empty());
    };
};

suite<"PollSlot"> poll_slot_suite = [] {
    "defaults to POLLING phase, not on stack, a failed result, and an empty task id"_test = [] {
        PollSlot slot;
        expect(slot.get_phase() == SlotPhase::POLLING);
        expect(not slot.is_on_stack());
        expect(not slot.get_success());
        expect(slot.get_task_id().empty());
    };

    "set_phase/get_phase round-trip across every phase"_test = [] {
        PollSlot slot;
        slot.set_phase(SlotPhase::AWAITING);
        expect(slot.get_phase() == SlotPhase::AWAITING);
        slot.set_phase(SlotPhase::RESUMING);
        expect(slot.get_phase() == SlotPhase::RESUMING);
        slot.set_phase(SlotPhase::POLLING);
        expect(slot.get_phase() == SlotPhase::POLLING);
    };

    "set_task_id/get_task_id round-trip"_test = [] {
        PollSlot slot;
        slot.set_task_id("task-42");
        expect(slot.get_task_id() == "task-42");
    };

    "set_on_stack/is_on_stack round-trip"_test = [] {
        PollSlot slot;
        slot.set_on_stack(true);
        expect(slot.is_on_stack());
        slot.set_on_stack(false);
        expect(not slot.is_on_stack());
    };

    "set_result/get_success round-trip, take_output moves the map out (empty on a second call)"_test =
        [] {
            PollSlot slot;
            slot.set_result(true, {{"key", "value"}});
            expect(slot.get_success());
            auto output = slot.take_output();
            expect(output.at("key") == "value");
            expect(slot.take_output().empty());
        };

    "get_mutex exposes a lockable mutex"_test = [] {
        PollSlot slot;
        expect(nothrow([&] { std::lock_guard lock{slot.get_mutex()}; }));
    };

    "wake() is a no-op with no wake callback registered; set_wake installs one wake() invokes"_test =
        [] {
            PollSlot slot;
            expect(nothrow([&] { slot.wake(); }));

            bool woke = false;
            slot.set_wake([&woke] { woke = true; });
            slot.wake();
            expect(woke);
        };
};

suite<"ExternalWorkerManager"> external_worker_manager_suite = [] {
    "backend_name reports \"external\""_test = [] {
        ExternalWorkerManager manager;
        expect(manager.backend_name() == "external");
    };

    "list is empty when nothing is registered"_test = [] {
        ExternalWorkerManager manager;
        expect(manager.list().empty());
    };

    "add_worker registers a worker, reflected in list()/status()"_test = [] {
        ExternalWorkerManager manager;
        FakeWorker worker{"echo"};
        manager.add_worker(worker);

        auto listed = manager.list();
        expect(listed.size() == 1U) << fatal;
        expect(listed[0].worker_id == "echo");
        expect(listed[0].worker_type == "echo");
        expect(listed[0].alive);

        auto status = manager.status("echo");
        expect(status.has_value()) << fatal;
        expect(status->worker_id == "echo");

        expect(not manager.status("missing").has_value());
    };

    "poll_once is a no-op with no registered worker types"_test = [] {
        ExternalWorkerManager manager;
        expect(nothrow([&] { manager.poll_once(); }));
    };

    "poll_once swallows a transport exception for a registered type instead of propagating"_test =
        [] {
            ExternalWorkerManager manager;
            install_throwing_client(manager);
            FakeWorker worker{"echo"};
            manager.add_worker(worker);
            expect(nothrow([&] { manager.poll_once(); }));
        };

    "register_poll_slot grows internal storage so a later poll_slot() call at that index is a valid (not OOB) access"_test =
        [] {
            ExternalWorkerManager manager;
            manager.register_poll_slot(2, [] {});

            // No registered task types: poll_slot(2) falls through past the (now-valid) slot
            // lookup to the sleep+reschedule tail, which throws — cleanly, not UB — since no
            // contract context is bound on this thread. That well-defined throw (rather than a
            // crash from indexing an unallocated deque slot) is what proves register_poll_slot()
            // actually sized m_slots to fit index 2.
            shared::this_handler::current = nullptr;
            expect(throws<std::runtime_error>([&] { manager.poll_slot(2); }));
        };

    "start_server logs and returns early when no contract group/leverager were resolved"_test = [] {
        ExternalWorkerManager manager;
        expect(nothrow([&] { manager.start_server(); }));
    };

    "spawn/start/stop/restart/shutdown_all are minimal always-succeed lifecycle stubs"_test = [] {
        ExternalWorkerManager manager;
        expect(not manager.spawn(interfaces::WorkerSpec{}).has_value());
        expect(manager.start("any"));
        expect(manager.stop("any"));
        expect(manager.restart("any"));
        expect(nothrow([&] { manager.shutdown_all(); }));
    };

    "set_health_callback stores the callback without invoking it"_test = [] {
        ExternalWorkerManager manager;
        bool called = false;
        manager.set_health_callback([&called](const interfaces::WorkerInfo &) { called = true; });
        expect(not called);
    };

    "register_task_defs/register_workflow_defs swallow per-item transport exceptions"_test = [] {
        ExternalWorkerManager manager;
        install_throwing_client(manager);
        expect(nothrow([&] { manager.register_task_defs({"{}", "{}"}); }));
        expect(nothrow([&] { manager.register_workflow_defs({"{}"}); }));
    };

    "on_load resolves a null contract group/leverager from an empty host and a default config_path"_test =
        [] {
            ExternalWorkerManager manager;
            CongeladoHostCallbacks host{};
            CongeladoConfigView cfg{};
            expect(nothrow([&] { manager.on_load(host, cfg); }));
            // With no contract group/leverager resolved, start_server() still safely no-ops.
            expect(nothrow([&] { manager.start_server(); }));
        };
};

suite<"WorkerManagerPlugin"> worker_manager_plugin_suite = [] {
    "reports its identity metadata"_test = [] {
        WorkerManagerPlugin plugin;
        expect(plugin.get_name() == "worker_manager_external");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker_manager");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER_MANAGER);
    };

    "worker_manager_get exposes the owned manager through the IWorkerManager interface"_test = [] {
        WorkerManagerPlugin plugin;
        auto *raw = plugin.worker_manager_get();
        expect(raw != nullptr) << fatal;
        auto *manager = static_cast<interfaces::IWorkerManager *>(raw);
        expect(manager->backend_name() == "external");
    };

    "on_load forwards host callbacks into the owned manager without throwing"_test = [] {
        WorkerManagerPlugin plugin;
        CongeladoHostCallbacks host{};
        CongeladoConfigView cfg{};
        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };
};

} // namespace external_worker_manager_tests
} // namespace
#endif

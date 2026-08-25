module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rfl/json.hpp>

export module echo_worker_plugin;

import congelado_plugin;
import interfaces;
import core_contract;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The simplest worker: copies every input key/value pair straight back out — nested/non-
/// string values JSON-stringified via `serde::Ser::encode_json`, so the flat output map survives.
/// Good for smoke-testing the task pipeline end to end. Now a first-class `interfaces::IWorker`
/// (dynamic Value in, flat map out) instead of the old CONGELADO_TASK duck-typed ABI.
class EchoWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "echo"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        interfaces::WorkerOutput output;
        if (auto object = input.to_object()) {
            for (auto const &[key, value] : *object) {
                if (auto as_string = value.to_string()) {
                    output[key] = *as_string;
                } else {
                    output[key] = serde::Ser::encode_json(value);
                }
            }
        }
        return output;
    }
};

/// @brief The echo worker plugin — exports the WORKER capability backed by EchoWorker.
class EchoWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "echo_worker"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "worker"; }

    /**
     * @brief Flags this as worker-capable, so the host wires `worker_get` into the `_cap_dispatch`
     * routing and resolves this plugin's IWorker* for the worker manager.
     * @return `CONGELADO_CAP_WORKER`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER;
    }

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        if (auto *group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
            group != nullptr) {
            m_worker.set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IWorker` surface.
     * @return this plugin's EchoWorker, upcast to `interfaces::IWorker*`.
     */
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    EchoWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(EchoWorkerPlugin);

#ifdef CONGELADO_TEST
namespace echo_worker_plugin_tests {
using namespace boost::ut;

suite<"EchoWorker"> echo_worker_suite = [] {
    "get_task_type reports 'echo'"_test = [] {
        EchoWorker worker;
        expect(worker.get_task_type() == "echo");
    };

    "execute copies a string field through unchanged"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"greeting":"hello"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("greeting") == "hello");
    };

    "execute copies multiple string fields through unchanged"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"a":"1","b":"2"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->size() == 2);
        expect(result->at("a") == "1");
        expect(result->at("b") == "2");
    };

    "execute JSON-encodes a non-string (number) field"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"count":42})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("count") == "42");
    };

    "execute JSON-encodes a non-string (bool) field"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"flag":true})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("flag") == "true");
    };

    "execute JSON-encodes a nested object field"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"nested":{"x":1}})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("nested") == R"({"x":1})");
    };

    "execute JSON-encodes a nested array field"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"items":[1,2,3]})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("items") == "[1,2,3]");
    };

    "execute on an empty object returns an empty output map"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->empty());
    };

    // Adversarial: input that isn't a JSON object at all (to_object() returns nullopt) — the
    // worker must not crash, it just has nothing to copy.
    "execute on a non-object input value returns an empty output map, no crash"_test = [] {
        EchoWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"("just a string")").value();
        expect(nothrow([&] {
            auto result = worker.execute(value);
            expect(result.has_value()) << fatal;
            expect(result->empty());
        }));
    };

    // Adversarial: a large number of keys — echo has no cap on how many fields it will mirror
    // back, but this stays at a moderate size to keep the test fast and deterministic.
    "execute handles a wide object with many keys"_test = [] {
        EchoWorker worker;
        std::string json = "{";
        for (int index = 0; index < 200; ++index) {
            if (index != 0) {
                json += ",";
            }
            json += std::format(R"("k{}":"v{}")", index, index);
        }
        json += "}";
        auto value = rfl::json::read<rfl::Generic>(json).value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->size() == 200);
        expect(result->at("k0") == "v0");
        expect(result->at("k199") == "v199");
    };
};

suite<"EchoWorkerPlugin"> echo_worker_plugin_suite = [] {
    "get_name reports 'echo_worker'"_test = [] {
        EchoWorkerPlugin plugin;
        expect(plugin.get_name() == "echo_worker");
    };

    "get_version reports '1.0.0'"_test = [] {
        EchoWorkerPlugin plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'worker'"_test = [] {
        EchoWorkerPlugin plugin;
        expect(plugin.get_unique_type() == "worker");
    };

    "capabilities reports CONGELADO_CAP_WORKER"_test = [] {
        EchoWorkerPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'echo'"_test = [] {
        EchoWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "echo");
    };

    "on_load with an empty host/config view does not crash"_test = [] {
        EchoWorkerPlugin plugin;
        expect(nothrow(
            [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        EchoWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace echo_worker_plugin_tests
#endif

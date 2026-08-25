module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rfl/json.hpp>

export module transform_worker_plugin;

import congelado_plugin;
import interfaces;
import core_contract;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/// @brief Typed input for the `transform` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<TransformInput>` specialization below.
/// `transform` defaults to "identity" via its in-class member initializer.
class TransformInput {
  public:
    void setTransform(std::string value) { m_transform = std::move(value); }
    void setValue(std::string value) { m_value = std::move(value); }

    [[nodiscard]] const std::string &getTransform() const noexcept { return m_transform; }
    [[nodiscard]] const std::string &getValue() const noexcept { return m_value; }

  private:
    // BUG: this in-class default is dead for task input parsed via `serde::Ser::from_value` — that
    // path decodes the whole reflected NamedTuple at once, and a plain (non-`std::optional`) field
    // with no matching key in the input Value fails the ENTIRE decode ("Field named 'transform' not
    // found"), never reaching this default. See the pinning test below ("BUG: from_value fails
    // entirely when 'transform' is omitted...").
    std::string m_transform{"identity"};
    std::string m_value;
};

template <>
struct serde::Serializable<TransformInput> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"transform", &TransformInput::getTransform,
                             &TransformInput::setTransform>{},
            serde::FieldDesc<"value", &TransformInput::getValue, &TransformInput::setValue>{},
        };
    }
};

namespace {

/// @brief Applies a named string transform (`to_upper`, `to_lower`, `negate`, or identity
/// passthrough) to the `value` input field and returns it under `result`. Now a first-class
/// `interfaces::IWorker` instead of the old CONGELADO_TASK duck-typed ABI.
class TransformWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "transform"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        auto parsed = serde::Ser::from_value<TransformInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }
        const std::string &transform_type = parsed->getTransform();
        const std::string &value = parsed->getValue();

        std::string result;
        if (transform_type == "to_upper") {
            result = value;
            for (auto &character : result) {
                character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
            }
        } else if (transform_type == "to_lower") {
            result = value;
            for (auto &character : result) {
                character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
        } else if (transform_type == "negate") {
            // `negate` runs `value` through strtod; a non-numeric value quietly becomes 0.0.
            auto numeric = std::strtod(value.c_str(), nullptr);
            result = std::format("{}", -numeric);
        } else {
            result = value;
        }

        return interfaces::WorkerOutput{{"result", std::move(result)}};
    }
};

/// @brief The transform worker plugin — exports the WORKER capability backed by TransformWorker.
class TransformWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "transform_worker"; }
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
     * @return this plugin's TransformWorker, upcast to `interfaces::IWorker*`.
     */
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    TransformWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(TransformWorkerPlugin);

#ifdef CONGELADO_TEST
namespace transform_worker_plugin_tests {
using namespace boost::ut;

suite<"TransformInput"> transform_input_suite = [] {
    "setTransform/getTransform round-trips"_test = [] {
        TransformInput input;
        input.setTransform("to_upper");
        expect(input.getTransform() == "to_upper");
    };

    "setValue/getValue round-trips"_test = [] {
        TransformInput input;
        input.setValue("hello");
        expect(input.getValue() == "hello");
    };

    "default-constructed transform is 'identity'"_test = [] {
        TransformInput input;
        expect(input.getTransform() == "identity");
    };

    "default-constructed value is empty"_test = [] {
        TransformInput input;
        expect(input.getValue().empty());
    };

    // BUG: pins the finding documented above TransformInput's m_transform member — omitting
    // "transform" from the input Value fails the whole decode instead of falling back to the
    // documented "identity" default.
    "BUG: from_value fails entirely when 'transform' is omitted, despite its documented default"_test =
        [] {
            auto value = rfl::json::read<rfl::Generic>(R"({"value":"hi"})").value();
            auto parsed = serde::Ser::from_value<TransformInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("transform")) << parsed.error();
        };

    "from_value fails entirely when 'value' is omitted"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"transform":"identity"})").value();
        auto parsed = serde::Ser::from_value<TransformInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("value")) << parsed.error();
    };

    "from_value succeeds when both fields are present"_test = [] {
        auto value =
            rfl::json::read<rfl::Generic>(R"({"transform":"to_upper","value":"hi"})").value();
        auto parsed = serde::Ser::from_value<TransformInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getTransform() == "to_upper");
        expect(parsed->getValue() == "hi");
    };
};

suite<"TransformWorker"> transform_worker_suite = [] {
    "get_task_type reports 'transform'"_test = [] {
        TransformWorker worker;
        expect(worker.get_task_type() == "transform");
    };

    "execute upper-cases the value for to_upper"_test = [] {
        TransformWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"transform":"to_upper","value":"Hello World"})")
                .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "HELLO WORLD");
    };

    "execute lower-cases the value for to_lower"_test = [] {
        TransformWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"transform":"to_lower","value":"Hello World"})")
                .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "hello world");
    };

    "execute negates a numeric value"_test = [] {
        TransformWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"transform":"negate","value":"3.5"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "-3.5");
    };

    "execute negates a negative numeric value back to positive"_test = [] {
        TransformWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"transform":"negate","value":"-2"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "2");
    };

    // Pins the documented "negate silently becomes 0.0 for non-numeric input" behavior — no
    // error is raised for garbage input, it's just silently coerced.
    "negate silently treats non-numeric input as 0"_test = [] {
        TransformWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"transform":"negate","value":"not-a-number"})")
                .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "-0");
    };

    "execute passes value through unchanged for an unrecognized transform (identity fallback)"_test =
        [] {
            TransformWorker worker;
            auto value =
                rfl::json::read<rfl::Generic>(R"({"transform":"identity","value":"as-is"})")
                    .value();
            auto result = worker.execute(value);
            expect(result.has_value()) << fatal;
            expect(result->at("result") == "as-is");
        };

    "execute passes value through unchanged for a totally unknown transform name"_test = [] {
        TransformWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"transform":"bogus","value":"as-is"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("result") == "as-is");
    };

    "execute propagates the from_value error when a required field is missing"_test = [] {
        TransformWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"value":"hi"})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());
    };
};

suite<"TransformWorkerPlugin"> transform_worker_plugin_suite = [] {
    "get_name reports 'transform_worker'"_test = [] {
        TransformWorkerPlugin plugin;
        expect(plugin.get_name() == "transform_worker");
    };

    "get_version reports '1.0.0'"_test = [] {
        TransformWorkerPlugin plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'worker'"_test = [] {
        TransformWorkerPlugin plugin;
        expect(plugin.get_unique_type() == "worker");
    };

    "capabilities reports CONGELADO_CAP_WORKER"_test = [] {
        TransformWorkerPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'transform'"_test = [] {
        TransformWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "transform");
    };

    "on_load with an empty host/config view does not crash"_test = [] {
        TransformWorkerPlugin plugin;
        expect(nothrow(
            [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        TransformWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace transform_worker_plugin_tests
#endif

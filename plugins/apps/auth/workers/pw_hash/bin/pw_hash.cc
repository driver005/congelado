module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module auth_pw_hash_worker_plugin;

import congelado_plugin;
import interfaces;
import core_contract;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/// @brief Typed input for the `pw_hash` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<PasswordHashInput>` specialization below.
class PasswordHashInput {
  public:
    void setPassword(std::string value) { m_password = std::move(value); }

    [[nodiscard]] const std::string &getPassword() const noexcept { return m_password; }

  private:
    std::string m_password;
};

template <>
struct serde::Serializable<PasswordHashInput> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"password", &PasswordHashInput::getPassword,
                             &PasswordHashInput::setPassword>{},
        };
    }
};

namespace {

/// @brief Hashes a password into a stable digest — the auth app's "generate a derived credential"
/// primitive, expressed as a worker. Reads `password` from the input map and returns
/// `password_hash`. Uses std::hash purely as a deterministic stand-in; a real deployment would swap
/// in a proper KDF worker (argon2/bcrypt) with the same map-in/map-out shape.
class PasswordHashWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "pw_hash"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        auto parsed = serde::Ser::from_value<PasswordHashInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }
        // SECURITY: no empty/minimum-length check on the password before it's hashed. An input
        // whose "password" key is present but set to "" parses cleanly (from_value only rejects
        // a *missing* key, via reflect-cpp's field-presence requirement — that's an accident of
        // the deserializer, not a deliberate validation rule, and it doesn't cover an explicit
        // empty string) and produces a normal-looking, fixed password_hash for the empty
        // password. Nothing in this worker stops an empty (or otherwise trivially weak)
        // credential from being derived and stored unless some other layer upstream rejects it
        // first.
        // SECURITY: this is not a password KDF. std::hash is a fast, unsalted, non-cryptographic
        // hash — the opposite of what password storage needs (bcrypt/argon2/scrypt/PBKDF2: slow,
        // per-user-salted, tunable work factor). Consequences: (1) no salt means two users with
        // the same password get the identical "hash", instantly revealing that fact and making
        // this trivially rainbow-table-able; (2) the output is only 64 bits (16 hex chars) — a
        // birthday-bound brute force is cheap even ignoring speed; (3) std::hash runs in
        // nanoseconds, so there is no cost to an offline guessing attack the way there would be
        // with a deliberately slow KDF. Needs a real password-hashing library before this
        // touches anything resembling a real credential.
        auto digest = std::hash<std::string>{}(parsed->getPassword());
        return interfaces::WorkerOutput{{"password_hash", std::format("{:016x}", digest)}};
    }
};

/// @brief The password-hash worker plugin — exports the WORKER capability backed by
/// PasswordHashWorker.
class PasswordHashWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "auth_pw_hash_worker";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER;
    }

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        if (auto *group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
            group != nullptr) {
            m_worker.set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorker surface.
    /// @return this plugin's PasswordHashWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    PasswordHashWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(PasswordHashWorkerPlugin);

#ifdef CONGELADO_TEST
namespace auth_pw_hash_worker_plugin_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"PasswordHashInput"> pw_hash_input_suite = [] {
    "defaults to an empty password"_test = [] {
        PasswordHashInput input;
        expect(input.getPassword().empty());
    };

    "setPassword/getPassword round-trip"_test = [] {
        PasswordHashInput input;
        input.setPassword("hunter2");
        expect(input.getPassword() == "hunter2");
    };
};

suite<"PasswordHashWorker"> pw_hash_worker_suite = [] {
    "get_task_type reports 'pw_hash'"_test = [] {
        PasswordHashWorker worker;
        expect(worker.get_task_type() == "pw_hash");
    };

    "execute() with a password produces a 16-lowercase-hex-digit digest"_test = [] {
        PasswordHashWorker worker;
        auto result = worker.execute(make_value(R"({"password":"hunter2"})"));

        expect(result.has_value()) << fatal;
        auto hash = result->at("password_hash");
        expect(hash.size() == 16);
        expect(std::ranges::all_of(
            hash, [](char character) { return std::isxdigit(static_cast<unsigned char>(character)) != 0; }));
    };

    "execute() on input that isn't an object reports a parse error"_test = [] {
        PasswordHashWorker worker;
        auto result = worker.execute(make_value("[1,2,3]"));
        expect(!result.has_value());
    };

    // SECURITY: definitive proof of the weak-KDF finding above — this is the exact behavior a
    // real password hasher (bcrypt/argon2/scrypt/PBKDF2) must NOT have. Every one of those
    // generates a fresh random salt per call and embeds it in the output, so hashing the SAME
    // password twice yields two DIFFERENT stored values. Here it doesn't.
    "hashing the identical password twice produces the identical hash — no per-call salt exists"_test =
        [] {
            PasswordHashWorker worker_a;
            PasswordHashWorker worker_b;

            auto result_a = worker_a.execute(make_value(R"({"password":"correct horse battery staple"})"));
            auto result_b = worker_b.execute(make_value(R"({"password":"correct horse battery staple"})"));

            expect(result_a.has_value()) << fatal;
            expect(result_b.has_value()) << fatal;
            expect(result_a->at("password_hash") == result_b->at("password_hash"));
        };

    // SECURITY: an attacker never needs to call this worker at all — std::hash is a public
    // standard-library function, so anyone can reproduce the exact "hash" for a guessed
    // password outside this service entirely, with no secret key or access of any kind.
    "the stored hash is exactly std::hash<std::string>(password) formatted as 16 lowercase hex digits — independently reproducible offline"_test =
        [] {
            PasswordHashWorker worker;
            auto result = worker.execute(make_value(R"({"password":"correct horse battery staple"})"));
            expect(result.has_value()) << fatal;
            auto expected = std::format(
                "{:016x}", std::hash<std::string>{}("correct horse battery staple"));
            expect(result->at("password_hash") == expected);
        };

    // SECURITY: pins the finding in the SECURITY comment above execute()'s password-validation
    // gap — an explicit empty-string password (the "password" key present, its value "") is
    // NOT rejected. It parses cleanly and hashes just like any other password, proving there is
    // no empty/minimum-length check anywhere in this worker.
    "an explicit empty-string password (key present, value \"\") still produces a fixed, reproducible hash rather than being rejected — no empty-password validation exists"_test =
        [] {
            PasswordHashWorker worker;
            auto result = worker.execute(make_value(R"({"password":""})"));

            expect(result.has_value()) << fatal;
            auto expected = std::format("{:016x}", std::hash<std::string>{}(""));
            expect(result->at("password_hash") == expected);
        };

    // Unlike an explicit empty string above, omitting the "password" key entirely IS rejected —
    // but not by any password-strength logic. execute() decodes via serde::Ser::from_value,
    // which goes through rfl::from_generic; reflect-cpp's NamedTuple decode from a generic tree
    // requires every reflected field to be present in the source object, so a missing key is a
    // hard parse error here, same mechanism documented on the jwt_sign and db_query workers'
    // equivalent tests. This is an accident of the deserializer, not a deliberate "reject empty
    // passwords" guard — see the SECURITY comment above for the actual (missing) validation gap.
    "a password object with the 'password' key entirely omitted fails to parse, unlike an explicit empty string"_test =
        [] {
            PasswordHashWorker worker;
            auto result = worker.execute(make_value("{}"));

            expect(!result.has_value()) << fatal;
            expect(result.error().getMessage().contains("password"));
            expect(result.error().getMessage().contains("not found"));
        };
};

suite<"PasswordHashWorkerPlugin"> pw_hash_worker_plugin_suite = [] {
    "identity/capabilities are the declared pw-hash-worker surface"_test = [] {
        PasswordHashWorkerPlugin plugin;

        expect(plugin.get_name() == "auth_pw_hash_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get() exposes the same PasswordHashWorker instance via IWorker*"_test = [] {
        PasswordHashWorkerPlugin plugin;
        auto *worker = static_cast<interfaces::IWorker *>(plugin.worker_get());

        expect(worker != nullptr) << fatal;
        expect(worker->get_task_type() == "pw_hash");
    };

    "on_load with no controller_ctx (host wiring absent) doesn't crash and the worker still hashes"_test =
        [] {
            PasswordHashWorkerPlugin plugin;
            CongeladoHostCallbacks host{};
            CongeladoConfigView cfg{};

            plugin.on_load(host, cfg);

            auto *worker = static_cast<interfaces::IWorker *>(plugin.worker_get());
            auto result = worker->execute(make_value(R"({"password":"post-load"})"));
            expect(result.has_value()) << fatal;
            expect(result->at("password_hash").size() == 16);
        };
};

} // namespace auth_pw_hash_worker_plugin_tests
#endif

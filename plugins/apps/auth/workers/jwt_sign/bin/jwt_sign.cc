module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module auth_jwt_sign_worker_plugin;

import congelado_plugin;
import interfaces;
import core_contract;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/// @brief Typed input for the `jwt_sign` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<JwtSignInput>` specialization below. `sub`
/// defaults to "anonymous" via its in-class member initializer.
class JwtSignInput {
  public:
    void setSub(std::string value) { m_sub = std::move(value); }

    [[nodiscard]] const std::string &getSub() const noexcept { return m_sub; }

  private:
    std::string m_sub{"anonymous"};
};

template <>
struct serde::Serializable<JwtSignInput> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"sub", &JwtSignInput::getSub, &JwtSignInput::setSub>{},
        };
    }
};

namespace {

/// @brief Signs a token for a subject — the auth app's "issue a credential" primitive as a worker.
/// Reads `sub` from the input map (fed from the preceding hash node's output) and returns a `token`.
/// The token format is a deterministic stand-in, not a real JWS; a production worker would sign with
/// a key resolved through the host, same map-in/map-out shape.
class JwtSignWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "jwt_sign"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        auto parsed = serde::Ser::from_value<JwtSignInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }
        // SECURITY: not a real signature — no secret key of any kind is involved. `std::hash` is
        // a public, unkeyed, non-cryptographic function of `sub` alone, so anyone who knows (or
        // guesses) `sub` can compute the exact same "token" themselves with zero access to this
        // service; there's nothing here a verifier could check that an attacker couldn't forge.
        // It's also not JWT-shaped (no header/payload/signature, no `alg`), so there's no
        // `alg: none`-style bypass to worry about — this is simply unsigned data wearing a `jwt.`
        // prefix. std::hash is also implementation-defined and may not even be stable across
        // process restarts/binaries, which would additionally break verification if anything
        // ever tried to check these tokens.
        auto signature = std::hash<std::string>{}(parsed->getSub());
        return interfaces::WorkerOutput{
            {"token", std::format("jwt.{}.{:016x}", parsed->getSub(), signature)}};
    }
};

/// @brief The jwt-sign worker plugin — exports the WORKER capability backed by JwtSignWorker.
class JwtSignWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "auth_jwt_sign_worker";
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
    /// @return this plugin's JwtSignWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    JwtSignWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(JwtSignWorkerPlugin);

#ifdef CONGELADO_TEST
namespace auth_jwt_sign_worker_plugin_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"JwtSignInput"> jwt_sign_input_suite = [] {
    "defaults sub to 'anonymous'"_test = [] {
        JwtSignInput input;
        expect(input.getSub() == "anonymous");
    };

    "setSub/getSub round-trip"_test = [] {
        JwtSignInput input;
        input.setSub("user-42");
        expect(input.getSub() == "user-42");
    };
};

suite<"JwtSignWorker"> jwt_sign_worker_suite = [] {
    "get_task_type reports 'jwt_sign'"_test = [] {
        JwtSignWorker worker;
        expect(worker.get_task_type() == "jwt_sign");
    };

    "execute() with an explicit sub embeds it verbatim and appends a 16-hex-digit digest"_test = [] {
        JwtSignWorker worker;
        auto result = worker.execute(make_value(R"({"sub":"alice"})"));

        expect(result.has_value()) << fatal;
        auto token = result->at("token");
        expect(token.starts_with("jwt.alice."));
        auto digest = token.substr(token.rfind('.') + 1);
        expect(digest.size() == 16);
    };

    "execute() with no sub reports a parse error — from_value (rfl::from_generic) requires 'sub' present, so JwtSignInput's in-class 'anonymous' default is never reached from this path"_test =
        [] {
            JwtSignWorker worker;
            auto result = worker.execute(make_value("{}"));

            // JwtSignInput::m_sub defaults to "anonymous" when the class is default-constructed
            // directly, but execute() decodes via serde::Ser::from_value, which goes through
            // rfl::from_generic — reflect-cpp's NamedTuple decode from a generic tree requires
            // every reflected field to be present in the source object. An input object with no
            // "sub" key is therefore a hard parse error, not a silent fallback to the default.
            expect(!result.has_value()) << fatal;
            expect(result.error().getMessage().contains("sub"));
            expect(result.error().getMessage().contains("not found"));
        };

    "execute() on input that isn't an object reports a parse error"_test = [] {
        JwtSignWorker worker;
        auto result = worker.execute(make_value("[1,2,3]"));
        expect(!result.has_value());
    };

    // SECURITY: pins the finding in the SECURITY comment above execute()'s `std::hash` call —
    // the "signature" is a pure, unkeyed function of `sub`. Two independent workers (nothing
    // shared between them, no key material of any kind exists to share) produce byte-identical
    // tokens for the same `sub`, proving there is no secret gating token issuance: anyone who
    // can reach a `sub` string can compute a "valid" token without ever calling this worker.
    "two independent JwtSignWorker instances produce an identical token for the same sub — no per-instance/per-deployment secret is involved"_test =
        [] {
            JwtSignWorker worker_a;
            JwtSignWorker worker_b;

            auto result_a = worker_a.execute(make_value(R"({"sub":"victim@example.com"})"));
            auto result_b = worker_b.execute(make_value(R"({"sub":"victim@example.com"})"));

            expect(result_a.has_value()) << fatal;
            expect(result_b.has_value()) << fatal;
            expect(result_a->at("token") == result_b->at("token"));
        };

    // SECURITY: an attacker never needs to call this worker at all — the same std::hash the
    // worker uses is a public standard-library function, so replicating it outside the service
    // reproduces the exact "signed" token for any chosen sub.
    "the token's digest is exactly std::hash<std::string>(sub) formatted as 16 lowercase hex digits — independently reproducible with zero access to this worker or any secret"_test =
        [] {
            JwtSignWorker worker;
            auto result = worker.execute(make_value(R"({"sub":"forge-me"})"));

            expect(result.has_value()) << fatal;
            auto expected =
                std::format("jwt.forge-me.{:016x}", std::hash<std::string>{}("forge-me"));
            expect(result->at("token") == expected);
        };
};

suite<"JwtSignWorkerPlugin"> jwt_sign_worker_plugin_suite = [] {
    "identity/capabilities are the declared jwt-sign-worker surface"_test = [] {
        JwtSignWorkerPlugin plugin;

        expect(plugin.get_name() == "auth_jwt_sign_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get() exposes the same JwtSignWorker instance via IWorker*"_test = [] {
        JwtSignWorkerPlugin plugin;
        auto *worker = static_cast<interfaces::IWorker *>(plugin.worker_get());

        expect(worker != nullptr) << fatal;
        expect(worker->get_task_type() == "jwt_sign");
    };

    "on_load with no controller_ctx (host wiring absent) doesn't crash and the worker still signs"_test =
        [] {
            JwtSignWorkerPlugin plugin;
            CongeladoHostCallbacks host{};
            CongeladoConfigView cfg{};

            plugin.on_load(host, cfg);

            auto *worker = static_cast<interfaces::IWorker *>(plugin.worker_get());
            auto result = worker->execute(make_value(R"({"sub":"post-load"})"));
            expect(result.has_value()) << fatal;
            expect(result->at("token").starts_with("jwt.post-load."));
        };
};

} // namespace auth_jwt_sign_worker_plugin_tests
#endif

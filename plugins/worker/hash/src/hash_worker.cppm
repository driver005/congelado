module;

#include <openssl/evp.h>
#include <rfl/json.hpp>

export module hash_worker;

import std;
import interfaces;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_hash {

/// @brief Typed input for the `hash` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<HashInput>` specialization below. `algo` defaults
/// to "sha256" via its in-class member initializer.
class HashInput {
  public:
    void setData(std::string value) { m_data = std::move(value); }
    void setAlgo(std::string value) { m_algo = std::move(value); }

    [[nodiscard]] const std::string &getData() const noexcept { return m_data; }
    [[nodiscard]] const std::string &getAlgo() const noexcept { return m_algo; }

  private:
    std::string m_data;
    // BUG: this in-class default is dead for task input parsed via `serde::Ser::from_value` —
    // that path decodes the whole reflected NamedTuple at once, and a plain (non-`std::optional`)
    // field with no matching key in the input Value fails the ENTIRE decode ("Field named 'algo'
    // not found"), never reaching this default. See the pinning test in the CONGELADO_TEST block
    // below ("BUG: from_value fails entirely when 'algo' is omitted...").
    std::string m_algo{"sha256"};
};

} // namespace worker_hash

template <>
struct serde::Serializable<worker_hash::HashInput> {
    static constexpr auto fields() {
        using worker_hash::HashInput;
        return std::tuple{
            serde::FieldDesc<"data", &HashInput::getData, &HashInput::setData>{},
            serde::FieldDesc<"algo", &HashInput::getAlgo, &HashInput::setAlgo>{},
        };
    }
};

export namespace worker_hash {

/// @brief A "generating a digest is a worker" primitive — hashes `data` with OpenSSL's EVP digest
/// API and returns the lowercase hex digest. Shared/reusable (lives in plugins/worker/). Reads
/// `data` and an optional `algo` (any OpenSSL digest name, e.g. "sha256"/"sha512"/"md5"; defaults to
/// sha256) from the input map, returns `hash` + the resolved `algo`.
class HashWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "hash"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        auto parsed = serde::Ser::from_value<HashInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }
        std::string algo = parsed->getAlgo();

        const EVP_MD *digest = EVP_get_digestbyname(algo.c_str());
        if (digest == nullptr) {
            digest = EVP_sha256();
            algo = "sha256";
        }

        std::array<unsigned char, EVP_MAX_MD_SIZE> out{};
        unsigned int length = 0;
        if (EVP_Digest(parsed->getData().data(), parsed->getData().size(), out.data(), &length,
                       digest, nullptr) != 1) {
            return std::unexpected{interfaces::WorkerError{"digest failed"}};
        }

        std::string hex;
        hex.reserve(static_cast<std::size_t>(length) * 2);
        for (unsigned int index = 0; index < length; ++index) {
            hex += std::format("{:02x}", out[index]);
        }
        return interfaces::WorkerOutput{{"hash_status", "ok"}, {"hash", hex}, {"algo", algo}};
    }
};

} // namespace worker_hash

#ifdef CONGELADO_TEST
namespace worker_hash::hash_worker_tests {
using namespace boost::ut;

suite<"HashInput"> hash_input_suite = [] {
    "setData/getData round-trips"_test = [] {
        HashInput input;
        input.setData("payload");
        expect(input.getData() == "payload");
    };

    "setAlgo/getAlgo round-trips"_test = [] {
        HashInput input;
        input.setAlgo("md5");
        expect(input.getAlgo() == "md5");
    };

    "default-constructed algo is sha256"_test = [] {
        HashInput input;
        expect(input.getAlgo() == "sha256");
    };

    // BUG: the doc comment on HashInput claims `algo` "defaults to sha256 via its in-class
    // member initializer", implying a task input that omits "algo" still decodes fine. In
    // reality `serde::Ser::from_value` decodes every declared field of the reflected
    // NamedTuple unconditionally — a plain (non-`std::optional`) field with no key present in
    // the input Value fails the WHOLE decode, the in-class default is never reached. Any
    // caller relying on the documented "algo defaults to sha256 when omitted" behavior gets a
    // hard parse error instead.
    "BUG: from_value fails entirely when 'algo' is omitted, despite its documented default"_test =
        [] {
            auto value = rfl::json::read<rfl::Generic>(R"({"data":"abc"})").value();
            auto parsed = serde::Ser::from_value<HashInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("algo")) << parsed.error();
        };

    "from_value fails entirely when 'data' is omitted"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"algo":"sha256"})").value();
        auto parsed = serde::Ser::from_value<HashInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("data")) << parsed.error();
    };

    "from_value succeeds when every declared field is present"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"data":"abc","algo":"sha256"})").value();
        auto parsed = serde::Ser::from_value<HashInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getData() == "abc");
        expect(parsed->getAlgo() == "sha256");
    };
};

suite<"HashWorker"> hash_worker_suite = [] {
    "get_task_type reports 'hash'"_test = [] {
        HashWorker worker;
        expect(worker.get_task_type() == "hash");
    };

    "execute computes the correct sha256 hex digest"_test = [] {
        HashWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"data":"abc","algo":"sha256"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("hash_status") == "ok");
        expect(result->at("algo") == "sha256");
        expect(result->at("hash") ==
               "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    };

    "execute computes the correct md5 hex digest for a different algo"_test = [] {
        HashWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"data":"abc","algo":"md5"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("algo") == "md5");
        expect(result->at("hash") == "900150983cd24fb0d6963f7d28e17f72");
    };

    "execute hashes an empty data string cleanly"_test = [] {
        HashWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"data":"","algo":"sha256"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("hash") ==
               "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    };

    "execute falls back to sha256 for an unrecognized algo name"_test = [] {
        HashWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"data":"abc","algo":"not-a-real-digest"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("algo") == "sha256");
        expect(result->at("hash") ==
               "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    };

    "execute falls back to sha256 for an empty algo name"_test = [] {
        HashWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"data":"abc","algo":""})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("algo") == "sha256");
    };

    "execute propagates the from_value error when a required field is missing"_test = [] {
        HashWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"algo":"sha256"})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());
    };

    // Adversarial: attacker-controlled `data` is arbitrary bytes handed straight to
    // EVP_Digest — a long input exercises the digest path with no length cap in this worker.
    "execute handles a large data payload without truncation"_test = [] {
        HashWorker worker;
        std::string large(100000, 'x');
        auto value =
            rfl::json::read<rfl::Generic>(std::format(R"({{"data":"{}","algo":"sha256"}})", large))
                .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("hash").size() == 64);
    };
};

} // namespace worker_hash::hash_worker_tests
#endif

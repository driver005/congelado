module;

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <rfl/json.hpp>

export module jwt_worker;

import std;
import interfaces;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_jwt {

/// @brief Typed input for the `jwt` worker's fixed fields, parsed from the task's dynamic input value
/// via `serde::Ser::from_value` — see the `Serializable<JwtInput>` specialization below. Every OTHER
/// input key becomes a claim and doesn't fit a fixed field list, so `JwtWorker::execute` still scans
/// the raw input object for those.
class JwtInput {
  public:
    void setSecret(std::string value) { m_secret = std::move(value); }

    [[nodiscard]] const std::string &getSecret() const noexcept { return m_secret; }

  private:
    std::string m_secret;
};

} // namespace worker_jwt

template <>
struct serde::Serializable<worker_jwt::JwtInput> {
    static constexpr auto fields() {
        using worker_jwt::JwtInput;
        return std::tuple{
            serde::FieldDesc<"secret", &JwtInput::getSecret, &JwtInput::setSecret>{},
        };
    }
};

export namespace worker_jwt {

/// @brief The `jwt` worker — issues a signed JSON Web Token (Conductor's Get-Signed-JWT system task),
/// as a reusable IWorker. HS256 (HMAC-SHA256) via OpenSSL. Input: `secret` (the HMAC key) + every
/// other input key becomes a string claim in the payload (e.g. `sub`, `iss`, `exp`). Output: `token`
/// (the compact JWS) + `jwt_status`.
class JwtWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "jwt"; }

    [[nodiscard]] interfaces::WorkerResult
    execute(const serde::Value &input) override {
        auto parsed = serde::Ser::from_value<JwtInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }

        // Build the claims payload from every input key except the reserved ones — string values go
        // in raw (matching the old flat-map behavior), anything else as its JSON encoding.
        // SECURITY: string claim keys/values are spliced straight into the payload with NO JSON
        // escaping — a claim value containing an unescaped `"` breaks out of its string literal and
        // can inject additional sibling claims into the signed payload (e.g. a claim value of
        // `x","admin":"true` becomes two top-level keys, "name" and a forged "admin", in the signed
        // JSON). See the pinning test below ("SECURITY: unescaped claim values allow injecting
        // extra top-level JSON claims").
        std::string payload = "{";
        bool first = true;
        if (auto object = input.to_object()) {
            for (const auto &[key, value] : *object) {
                if (key == "secret" || key == "algo") {
                    continue;
                }
                if (!first) {
                    payload += ",";
                }
                if (auto as_string = value.to_string()) {
                    payload += "\"" + key + "\":\"" + *as_string + "\"";
                } else {
                    payload += "\"" + key + "\":" + serde::Ser::encode_json(value);
                }
                first = false;
            }
        }
        payload += "}";

        const std::string header = R"({"alg":"HS256","typ":"JWT"})";
        std::string signing_input = base64url(header) + "." + base64url(payload);

        std::array<unsigned char, EVP_MAX_MD_SIZE> mac{};
        unsigned int mac_length = 0;
        const std::string &secret = parsed->getSecret();
        // SECURITY: `secret` is never validated for non-emptiness or minimum strength — a task
        // supplying `"secret":""` still gets back a "successfully" HMAC-signed token (HMAC-SHA256
        // with a zero-length key is well-defined, not an error), indistinguishable from one signed
        // with a real key to any verifier that trusts this worker's output.
        if (HMAC(EVP_sha256(), secret.data(), static_cast<int>(secret.size()),
                 reinterpret_cast<const unsigned char *>(signing_input.data()),
                 signing_input.size(), mac.data(), &mac_length) == nullptr) {
            return std::unexpected{interfaces::WorkerError{"hmac failed"}};
        }
        std::string token =
            signing_input + "." + base64url_bytes(mac.data(), mac_length);
        return interfaces::WorkerOutput{{"jwt_status", "ok"}, {"token", std::move(token)}};
    }

  private:
    /// @brief Standard base64 of raw bytes via OpenSSL.
    static std::string base64(const unsigned char *data, std::size_t length) {
        std::string out;
        out.resize(4 * ((length + 2) / 3));
        int written = EVP_EncodeBlock(reinterpret_cast<unsigned char *>(out.data()), data,
                                      static_cast<int>(length));
        out.resize(static_cast<std::size_t>(written));
        return out;
    }

    /// @brief base64url (url-safe, no padding) of raw bytes.
    static std::string base64url_bytes(const unsigned char *data, std::size_t length) {
        std::string encoded = base64(data, length);
        for (auto &character : encoded) {
            if (character == '+') {
                character = '-';
            } else if (character == '/') {
                character = '_';
            }
        }
        while (!encoded.empty() && encoded.back() == '=') {
            encoded.pop_back();
        }
        return encoded;
    }

    /// @brief base64url of a string.
    static std::string base64url(std::string_view text) {
        return base64url_bytes(reinterpret_cast<const unsigned char *>(text.data()), text.size());
    }
};

} // namespace worker_jwt

#ifdef CONGELADO_TEST
namespace worker_jwt::jwt_worker_tests {
using namespace boost::ut;

/// @brief Test-only base64url decoder — the inverse of JwtWorker's own private base64url()
/// helper, needed so a test can re-parse a produced token segment's decoded JSON text. Kept as a
/// small class (not a free function), same convention as every other test helper in this repo.
class JwtSegmentDecoder {
  public:
    JwtSegmentDecoder() = delete;

    /// @brief base64url-decodes one dot-separated JWT segment back to raw text.
    /// @param encoded the base64url (no padding) segment to decode.
    /// @return the decoded bytes as a string, or empty on a decode failure.
    static std::string decode(std::string_view encoded) {
        std::string padded{encoded};
        for (auto &character : padded) {
            if (character == '-') {
                character = '+';
            } else if (character == '_') {
                character = '/';
            }
        }
        std::size_t missing_padding = (4 - (padded.size() % 4)) % 4;
        padded.append(missing_padding, '=');

        std::string decoded;
        decoded.resize(padded.size() / 4 * 3);
        int written = EVP_DecodeBlock(reinterpret_cast<unsigned char *>(decoded.data()),
                                      reinterpret_cast<const unsigned char *>(padded.data()),
                                      static_cast<int>(padded.size()));
        if (written < 0) {
            return {};
        }
        decoded.resize(static_cast<std::size_t>(written) - missing_padding);
        return decoded;
    }
};

suite<"JwtInput"> jwt_input_suite = [] {
    "setSecret/getSecret round-trips"_test = [] {
        JwtInput input;
        input.setSecret("topsecret");
        expect(input.getSecret() == "topsecret");
    };

    "default-constructed secret is empty"_test = [] {
        JwtInput input;
        expect(input.getSecret().empty());
    };

    "from_value fails entirely when 'secret' is omitted"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"sub":"alice"})").value();
        auto parsed = serde::Ser::from_value<JwtInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("secret")) << parsed.error();
    };

    "from_value succeeds when 'secret' is present"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"secret":"s"})").value();
        auto parsed = serde::Ser::from_value<JwtInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getSecret() == "s");
    };
};

suite<"JwtWorker"> jwt_worker_suite = [] {
    "get_task_type reports 'jwt'"_test = [] {
        JwtWorker worker;
        expect(worker.get_task_type() == "jwt");
    };

    "execute produces the exact expected HS256 token for a known secret/claim"_test = [] {
        JwtWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"secret":"mysecret","sub":"alice"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("jwt_status") == "ok");
        // Cross-checked against an independent Python hmac/base64url computation of the same
        // header + {"sub":"alice"} payload signed with "mysecret".
        expect(result->at("token") ==
               "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
               "eyJzdWIiOiJhbGljZSJ9."
               "R4VJ9s9VDROUi_ERIPpQISMfXY3Hd61Q2udlANOm6sc");
    };

    "execute excludes 'secret' and 'algo' from the signed claims payload"_test = [] {
        JwtWorker worker;
        auto value = rfl::json::read<rfl::Generic>(
                        R"({"secret":"s","algo":"ignored","sub":"bob"})")
                        .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        auto token = result->at("token");
        auto first_dot = token.find('.');
        auto second_dot = token.find('.', first_dot + 1);
        auto payload_segment = token.substr(first_dot + 1, second_dot - first_dot - 1);
        auto decoded_payload = JwtSegmentDecoder::decode(payload_segment);
        expect(!decoded_payload.contains("algo")) << decoded_payload;
        expect(decoded_payload.contains("bob")) << decoded_payload;
    };

    "execute with no extra claims signs an empty claims object"_test = [] {
        JwtWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"secret":"s"})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        auto token = result->at("token");
        auto first_dot = token.find('.');
        auto second_dot = token.find('.', first_dot + 1);
        auto payload_segment = token.substr(first_dot + 1, second_dot - first_dot - 1);
        expect(JwtSegmentDecoder::decode(payload_segment) == "{}");
    };

    "execute propagates the from_value error when 'secret' is missing"_test = [] {
        JwtWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"sub":"alice"})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());
    };

    // SECURITY: pins the finding documented above the payload-building loop in execute() —
    // an unescaped `"` inside a string claim value breaks out of its JSON string literal and
    // injects an unrelated sibling claim into the signed payload. Here a single input claim
    // ("name") ends up producing TWO claims in the signed JSON: the legitimate "name":"x" and a
    // forged "admin":"true" the caller never actually requested as a top-level claim.
    "SECURITY: unescaped claim values allow injecting extra top-level JSON claims"_test = [] {
        JwtWorker worker;
        auto value = rfl::json::read<rfl::Generic>(
                        R"({"secret":"s","name":"x\",\"admin\":\"true"})")
                        .value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;

        auto token = result->at("token");
        auto first_dot = token.find('.');
        auto second_dot = token.find('.', first_dot + 1);
        auto payload_segment = token.substr(first_dot + 1, second_dot - first_dot - 1);
        auto decoded_payload = JwtSegmentDecoder::decode(payload_segment);

        auto reparsed = rfl::json::read<rfl::Generic>(decoded_payload);
        expect(reparsed.has_value()) << fatal; // still syntactically valid JSON, just injected
        auto object = reparsed->to_object();
        expect(object.has_value()) << fatal;
        expect(object->size() == 2) << decoded_payload; // "name" + injected "admin"

        bool found_forged_admin = false;
        for (const auto &[key, forged_value] : *object) {
            if (key == "admin") {
                auto as_string = forged_value.to_string();
                found_forged_admin = as_string.has_value() && *as_string == "true";
            }
        }
        expect(found_forged_admin) << decoded_payload;
    };

    // SECURITY: pins the finding documented above the HMAC call in execute() — an empty
    // `secret` is accepted with no validation and still produces a "successfully" signed token.
    "SECURITY: an empty secret is accepted and still produces a signed token"_test = [] {
        JwtWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"secret":""})").value();
        auto result = worker.execute(value);
        expect(result.has_value()) << fatal;
        expect(result->at("jwt_status") == "ok");
        expect(!result->at("token").empty());
    };
};

} // namespace worker_jwt::jwt_worker_tests
#endif

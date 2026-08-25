module;

#include <curl/curl.h>

#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module email_worker;

import std;
import interfaces;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_email {

/// @brief Typed input for the `email` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<EmailInput>` specialization below for the field
/// list. `smtp_url` is required (checked after parsing); `username`/`password` default to empty.
class EmailInput {
  public:
    void setSmtpUrl(std::string value) { m_smtp_url = std::move(value); }
    void setFrom(std::string value) { m_from = std::move(value); }
    void setTo(std::string value) { m_to = std::move(value); }
    void setSubject(std::string value) { m_subject = std::move(value); }
    void setBody(std::string value) { m_body = std::move(value); }
    void setUsername(std::string value) { m_username = std::move(value); }
    void setPassword(std::string value) { m_password = std::move(value); }

    [[nodiscard]] const std::string &getSmtpUrl() const noexcept { return m_smtp_url; }
    [[nodiscard]] const std::string &getFrom() const noexcept { return m_from; }
    [[nodiscard]] const std::string &getTo() const noexcept { return m_to; }
    [[nodiscard]] const std::string &getSubject() const noexcept { return m_subject; }
    [[nodiscard]] const std::string &getBody() const noexcept { return m_body; }
    [[nodiscard]] const std::string &getUsername() const noexcept { return m_username; }
    [[nodiscard]] const std::string &getPassword() const noexcept { return m_password; }

  private:
    std::string m_smtp_url;
    std::string m_from;
    std::string m_to;
    std::string m_subject;
    std::string m_body;
    // BUG: same reflect-cpp gotcha documented on worker_hash::HashInput::m_algo — neither field
    // is `std::optional`, so `serde::Ser::from_value` requires BOTH present in the task input or
    // the WHOLE decode fails, despite the doc comment above claiming they "default to empty". A
    // task that omits either key (a very natural thing to do for anonymous/unauthenticated relay)
    // gets a hard parse error instead of the documented empty default.
    std::string m_username;
    std::string m_password;
};

} // namespace worker_email

template <>
struct serde::Serializable<worker_email::EmailInput> {
    static constexpr auto fields() {
        using worker_email::EmailInput;
        return std::tuple{
            serde::FieldDesc<"smtp_url", &EmailInput::getSmtpUrl, &EmailInput::setSmtpUrl>{},
            serde::FieldDesc<"from", &EmailInput::getFrom, &EmailInput::setFrom>{},
            serde::FieldDesc<"to", &EmailInput::getTo, &EmailInput::setTo>{},
            serde::FieldDesc<"subject", &EmailInput::getSubject, &EmailInput::setSubject>{},
            serde::FieldDesc<"body", &EmailInput::getBody, &EmailInput::setBody>{},
            serde::FieldDesc<"username", &EmailInput::getUsername, &EmailInput::setUsername>{},
            serde::FieldDesc<"password", &EmailInput::getPassword, &EmailInput::setPassword>{},
        };
    }
};

export namespace worker_email {

/// @brief The `email` worker — sends an email over SMTP via libcurl (Conductor's SendGrid/email
/// system task, generalized to any SMTP server). Reusable IWorker. Input: `smtp_url` (required, e.g.
/// `smtps://smtp.example.com:465`), `from`, `to`, `subject`, `body`, optional `username`/`password`.
/// Output: `email_status` ("ok"/"error") + `error`.
class EmailWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "email"; }

    /// @brief No async curl integration exists in this codebase — this runs on the worker's own
    /// dedicated TaskQueue contract (see IWorker::run), so the blocking `curl_easy_perform` call
    /// never holds up the caller or the shared contract pool beyond this one contract.
    void run(const serde::Value &input,
            interfaces::WorkerCompletion on_complete) override {
        auto parsed = serde::Ser::from_value<EmailInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }
        if (parsed->getSmtpUrl().empty()) {
            on_complete(std::unexpected{interfaces::WorkerError{"missing 'smtp_url'"}});
            return;
        }
        on_complete(send_blocking(parsed->getSmtpUrl(), parsed->getFrom(), parsed->getTo(),
                                  parsed->getSubject(), parsed->getBody(), parsed->getUsername(),
                                  parsed->getPassword()));
    }

  private:
    /// @brief The in-flight message body + how much has been handed to libcurl so far.
    struct Upload {
        std::string payload;
        std::size_t offset;
    };

    /// @brief The actual blocking SMTP send — runs on this worker's own dedicated contract.
    static interfaces::WorkerResult
    send_blocking(const std::string &smtp_url, const std::string &from, const std::string &to,
                 const std::string &subject, const std::string &body, const std::string &username,
                 const std::string &password) {
        static const int global_init = [] {
            curl_global_init(CURL_GLOBAL_DEFAULT);
            return 0;
        }();
        (void)global_init;

        // SECURITY: `to`/`from`/`subject` are attacker-influenceable task input, spliced directly
        // into a raw RFC-5322 header block with no CRLF stripping/validation. A `to` or `subject`
        // value containing "\r\nBcc: attacker@evil.example" (or any other header line) is classic
        // email header injection — it rides straight through into the message libcurl uploads.
        Upload upload{.payload = "To: " + to + "\r\nFrom: " + from + "\r\nSubject: " + subject +
                                 "\r\n\r\n" + body + "\r\n",
                      .offset = 0};

        CURL *curl = curl_easy_init();
        if (curl == nullptr) {
            return std::unexpected{interfaces::WorkerError{"curl init failed"}};
        }
        // SECURITY (SSRF-shaped): `smtp_url` is task input handed straight to CURLOPT_URL with no
        // allowlist/validation — a task can point this worker at any host:port reachable from the
        // process (internal SMTP relays, or any other service that speaks enough of the SMTP
        // handshake to not immediately error out), enabling internal network probing/relay abuse
        // via a zero-auth task submission. No host allowlist exists anywhere on this path.
        curl_easy_setopt(curl, CURLOPT_URL, smtp_url.c_str());
        if (!username.empty()) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + from + ">").c_str());
        curl_slist *recipients = curl_slist_append(nullptr, ("<" + to + ">").c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, &read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode result = curl_easy_perform(curl);
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            return std::unexpected{interfaces::WorkerError{curl_easy_strerror(result)}};
        }
        return interfaces::WorkerOutput{{"email_status", "ok"}};
    }

    /// @brief libcurl read callback — streams the message body to the SMTP upload.
    static std::size_t read_callback(char *buffer, std::size_t size, std::size_t count,
                                     void *userdata) {
        auto *upload = static_cast<Upload *>(userdata);
        std::size_t room = size * count;
        std::size_t remaining = upload->payload.size() - upload->offset;
        std::size_t chunk = std::min(room, remaining);
        if (chunk > 0) {
            std::copy_n(upload->payload.data() + upload->offset, chunk, buffer);
            upload->offset += chunk;
        }
        return chunk;
    }
};

} // namespace worker_email

// Testing notes: send_blocking()/read_callback() are private static members reachable ONLY through
// run() once smtp_url is non-empty — at which point run() unconditionally calls curl_easy_perform(),
// a real blocking network attempt with no injectable transport seam (unlike core_client's IProtocol
// seam used by the client/client_pool workers). There is no way to reach that code from a test
// without either a live SMTP endpoint or a real outbound connection attempt, both forbidden by this
// repo's test-safety rules. That's a legitimate, documented skip for send_blocking/read_callback
// specifically. Everything reachable WITHOUT triggering a network call — EmailInput parsing, and
// run()'s two guard clauses (missing 'smtp_url' key entirely, and present-but-empty) — is fully
// tested below.
#ifdef CONGELADO_TEST
namespace worker_email::email_worker_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"EmailInput"> email_input_suite = [] {
    "setSmtpUrl/getSmtpUrl round-trips"_test = [] {
        EmailInput input;
        input.setSmtpUrl("smtps://smtp.example.com:465");
        expect(input.getSmtpUrl() == "smtps://smtp.example.com:465");
    };

    "setFrom/getFrom round-trips"_test = [] {
        EmailInput input;
        input.setFrom("a@example.com");
        expect(input.getFrom() == "a@example.com");
    };

    "setTo/getTo round-trips"_test = [] {
        EmailInput input;
        input.setTo("b@example.com");
        expect(input.getTo() == "b@example.com");
    };

    "setSubject/getSubject round-trips"_test = [] {
        EmailInput input;
        input.setSubject("hi");
        expect(input.getSubject() == "hi");
    };

    "setBody/getBody round-trips"_test = [] {
        EmailInput input;
        input.setBody("body text");
        expect(input.getBody() == "body text");
    };

    "setUsername/getUsername round-trips"_test = [] {
        EmailInput input;
        input.setUsername("user");
        expect(input.getUsername() == "user");
    };

    "setPassword/getPassword round-trips"_test = [] {
        EmailInput input;
        input.setPassword("pass");
        expect(input.getPassword() == "pass");
    };

    "default-constructed fields are all empty"_test = [] {
        EmailInput input;
        expect(input.getSmtpUrl().empty());
        expect(input.getUsername().empty());
        expect(input.getPassword().empty());
    };

    "from_value fails entirely when 'smtp_url' is omitted"_test = [] {
        auto value =
            make_value(R"({"from":"a@x.com","to":"b@x.com","subject":"s","body":"b",)"
                       R"("username":"","password":""})");
        auto parsed = serde::Ser::from_value<EmailInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("smtp_url")) << parsed.error();
    };

    // BUG: pins the finding documented above EmailInput's m_username/m_password — omitting either
    // fails the whole decode despite the doc comment claiming they default to empty.
    "BUG: from_value fails entirely when 'username'/'password' are omitted, despite documented empty default"_test =
        [] {
            auto value = make_value(
                R"({"smtp_url":"smtps://x","from":"a@x.com","to":"b@x.com","subject":"s","body":"b"})");
            auto parsed = serde::Ser::from_value<EmailInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("username") || parsed.error().contains("password"))
                << parsed.error();
        };

    "from_value succeeds when every declared field is present"_test = [] {
        auto value = make_value(
            R"({"smtp_url":"smtps://x","from":"a@x.com","to":"b@x.com","subject":"s","body":"b",)"
            R"("username":"u","password":"p"})");
        auto parsed = serde::Ser::from_value<EmailInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getSmtpUrl() == "smtps://x");
        expect(parsed->getUsername() == "u");
        expect(parsed->getPassword() == "p");
    };

    // SECURITY pin: no CRLF/header-injection stripping anywhere in the DTO — pins the finding
    // documented above the raw header-block construction in send_blocking().
    "SECURITY: 'to' carrying embedded CRLF header lines round-trips through the DTO untouched"_test =
        [] {
            EmailInput input;
            input.setTo("victim@x.com\r\nBcc: attacker@evil.example");
            expect(input.getTo() == "victim@x.com\r\nBcc: attacker@evil.example");
        };
};

suite<"EmailWorker"> email_worker_suite = [] {
    "get_task_type reports 'email'"_test = [] {
        EmailWorker worker;
        expect(worker.get_task_type() == "email");
    };

    "run() fails with 'missing smtp_url' when 'smtp_url' is present but empty, never touches curl"_test =
        [] {
            EmailWorker worker;
            auto value = make_value(
                R"({"smtp_url":"","from":"a@x.com","to":"b@x.com","subject":"s","body":"b",)"
                R"("username":"","password":""})");
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};
            bool called = false;

            worker.run(value, [&](interfaces::WorkerResult result) {
                called = true;
                observed = std::move(result);
            });

            expect(called) << fatal;
            expect(!observed.has_value()) << fatal;
            expect(observed.error().getMessage() == "missing 'smtp_url'");
        };

    "run() propagates the from_value parse error when 'smtp_url' key is entirely absent"_test = [] {
        EmailWorker worker;
        auto value = make_value(R"({"from":"a@x.com","to":"b@x.com","subject":"s","body":"b",)"
                                R"("username":"","password":""})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};

        worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage().contains("smtp_url"));
    };
};

} // namespace worker_email::email_worker_tests
#endif

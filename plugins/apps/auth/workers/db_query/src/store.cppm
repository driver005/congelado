export module auth_db_query_store;

import std;
import connector;
import model;
import serde;
#ifdef CONGELADO_TEST
import interfaces;
import shared;
import boost.ut;
#endif

export namespace auth {

/// @brief Typed input for the `db_query` worker, parsed from the task's flat input map via
/// `serde::Ser::from_map` — see the `Serializable<DbQueryInput>` specialization below. `username`
/// defaults to "default_user" via its in-class member initializer.
class DbQueryInput {
  public:
    void setUsername(std::string value) { m_username = std::move(value); }
    void setPasswordHash(std::string value) { m_password_hash = std::move(value); }

    [[nodiscard]] const std::string &getUsername() const noexcept { return m_username; }
    [[nodiscard]] const std::string &getPasswordHash() const noexcept { return m_password_hash; }

  private:
    std::string m_username{"default_user"};
    std::string m_password_hash;
};

/// @brief The connector-touching half of the `db_query` worker, kept in a module TU so the plugin's
/// own `.cc` never imports `connector` directly (that import crashes clang's modules in a plugin
/// entry TU — the engine dodges it the same way). Casts the opaque `connector_ctx` the host injected
/// and runs a real upsert + read-back through it.
class UserStore {
  public:
    /// @brief Outcome callback for `store_and_read_async` — the read-back (username, password_hash)
    /// pair, or std::nullopt on any failure.
    using Completion = std::move_only_function<void(std::optional<std::pair<std::string, std::string>>)>;

    /**
     * @brief Upserts an AuthUser then reads it back through the injected connector — proving a "db
     * call is a worker" against whatever backend the worker host resolved (in-memory LocalStore when
     * no storage plugin is loaded, a real DB otherwise). Chains straight through the connector's own
     * callbacks — no thread ever blocks.
     * @param connector_ctx the host-injected `connector::Connector*`, as an opaque pointer.
     * @param username the user to store (the primary key).
     * @param password_hash the derived hash to store.
     * @param completion fired once with the read-back pair, or std::nullopt on failure.
     */
    static void store_and_read_async(void *connector_ctx, std::string username,
                                     std::string password_hash, Completion completion) {
        if (connector_ctx == nullptr) {
            completion(std::nullopt);
            return;
        }
        auto &conn = *static_cast<connector::Connector *>(connector_ctx);

        model::AuthUser user;
        user.set_username(username);
        user.set_password_hash(std::move(password_hash));

        auto shared_completion = std::make_shared<Completion>(std::move(completion));
        conn.upsert<model::AuthUser>(
            user, [connector_ctx, username, shared_completion](bool ok) {
                if (!ok) {
                    (*shared_completion)(std::nullopt);
                    return;
                }
                auto &connector_ref = *static_cast<connector::Connector *>(connector_ctx);
                connector_ref.find<model::AuthUser>(
                    username, [shared_completion](std::optional<model::AuthUser> found) {
                        if (!found) {
                            (*shared_completion)(std::nullopt);
                            return;
                        }
                        (*shared_completion)(
                            std::make_pair(found->get_username(), found->get_password_hash()));
                    });
            });
    }
};

} // namespace auth

template <>
struct serde::Serializable<auth::DbQueryInput> {
    static constexpr auto fields() {
        using auth::DbQueryInput;
        return std::tuple{
            serde::FieldDesc<"username", &DbQueryInput::getUsername, &DbQueryInput::setUsername>{},
            serde::FieldDesc<"password_hash", &DbQueryInput::getPasswordHash,
                             &DbQueryInput::setPasswordHash>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace auth_db_query_store_tests {
using namespace boost::ut;

/// @brief Always-miss cache stub — `connector::Connector` running local-only (no `IDatabase`)
/// still requires a non-null cache (`active_cache()` aborts the process otherwise), so this
/// exists purely to satisfy that precondition for the tests below.
class NullCache final : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "null_cache"; }
    void get(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void set(std::string_view, std::string_view, shared::QueryReadFn &&result) noexcept override {
        result("ok");
    }
    void remove(std::string_view, shared::QueryReadFn &&result) noexcept override { result("ok"); }
};

suite<"DbQueryInput"> db_query_input_suite = [] {
    "defaults to username 'default_user' and an empty password_hash"_test = [] {
        auth::DbQueryInput input;

        expect(input.getUsername() == "default_user");
        expect(input.getPasswordHash().empty());
    };

    "setters round-trip through their getters"_test = [] {
        auth::DbQueryInput input;
        input.setUsername("alice");
        input.setPasswordHash("$argon2id$v=19$salted-hash");

        expect(input.getUsername() == "alice");
        expect(input.getPasswordHash() == "$argon2id$v=19$salted-hash");
    };
};

suite<"Serializable<DbQueryInput>"> db_query_input_serde_suite = [] {
    "from_map populates both fields when present"_test = [] {
        auto result = serde::Ser::from_map<auth::DbQueryInput>(
            {{"username", "bob"}, {"password_hash", "hashval"}});

        expect(result.has_value()) << fatal;
        expect(result->getUsername() == "bob");
        expect(result->getPasswordHash() == "hashval");
    };

    "from_map leaves username at its 'default_user' default when the key is absent"_test = [] {
        auto result = serde::Ser::from_map<auth::DbQueryInput>({{"password_hash", "hashval"}});

        expect(result.has_value()) << fatal;
        expect(result->getUsername() == "default_user");
        expect(result->getPasswordHash() == "hashval");
    };
};

suite<"UserStore::store_and_read_async"> user_store_suite = [] {
    "a null connector_ctx completes with nullopt, no crash"_test = [] {
        std::optional<std::pair<std::string, std::string>> observed{std::make_pair("x", "y")};
        bool called = false;

        auth::UserStore::store_and_read_async(
            nullptr, "alice", "hash1",
            [&](std::optional<std::pair<std::string, std::string>> result) {
                called = true;
                observed = std::move(result);
            });

        expect(called) << fatal;
        expect(!observed.has_value());
    };

    "stores then reads back the exact username/password_hash through a local-only connector"_test = [] {
        NullCache cache;
        connector::Connector conn;
        conn.set_cache(&cache);

        std::optional<std::pair<std::string, std::string>> observed;
        bool called = false;
        auth::UserStore::store_and_read_async(
            &conn, "carol", "$2b$12$abcdefghijklmnopqrstuv",
            [&](std::optional<std::pair<std::string, std::string>> result) {
                called = true;
                observed = std::move(result);
            });

        expect(called) << fatal;
        expect(observed.has_value()) << fatal;
        expect(observed->first == "carol");
        expect(observed->second == "$2b$12$abcdefghijklmnopqrstuv");
    };

    "a second store for the same username overwrites the prior password_hash (upsert semantics)"_test = [] {
        NullCache cache;
        connector::Connector conn;
        conn.set_cache(&cache);

        std::optional<std::pair<std::string, std::string>> first;
        auth::UserStore::store_and_read_async(&conn, "dave", "hash-v1",
                                              [&](auto result) { first = std::move(result); });
        std::optional<std::pair<std::string, std::string>> second;
        auth::UserStore::store_and_read_async(&conn, "dave", "hash-v2",
                                              [&](auto result) { second = std::move(result); });

        expect(first.has_value()) << fatal;
        expect(second.has_value()) << fatal;
        expect(first->second == "hash-v1");
        expect(second->second == "hash-v2");
    };

    "a SQL-injection-shaped username round-trips as inert data — this path never builds SQL text from it (local-only mode is a plain in-memory map, not a query)"_test = [] {
        NullCache cache;
        connector::Connector conn;
        conn.set_cache(&cache);

        std::string hostile_username = "robert'); DROP TABLE auth_users; --";
        std::optional<std::pair<std::string, std::string>> observed;
        auth::UserStore::store_and_read_async(&conn, hostile_username, "hash",
                                              [&](auto result) { observed = std::move(result); });

        expect(observed.has_value()) << fatal;
        expect(observed->first == hostile_username);
    };

    // SECURITY: pins what actually crosses back out of this layer. The read-back pair's
    // `.second` is the raw, unredacted password_hash — db_query.cc's DbQueryWorker::run() takes
    // this exact value and puts it straight into the task's WorkerOutput under "stored_hash"
    // (see the SECURITY comment there), and taskdefs/db_query.json declares "stored_hash" as a
    // plain output_key with an empty masked_fields list — so the derived credential a
    // downstream caller/API consumer would see when inspecting that task's result is the same
    // value that was persisted, not a placeholder/redaction.
    "the read-back pair exposes the raw password_hash unmodified — the exact value db_query.cc's run() forwards into WorkerOutput's unmasked 'stored_hash' key"_test =
        [] {
            NullCache cache;
            connector::Connector conn;
            conn.set_cache(&cache);

            std::optional<std::pair<std::string, std::string>> observed;
            auth::UserStore::store_and_read_async(&conn, "erin", "super-secret-derived-hash",
                                                  [&](auto result) { observed = std::move(result); });

            expect(observed.has_value()) << fatal;
            expect(observed->second == "super-secret-derived-hash");
        };
};

} // namespace auth_db_query_store_tests
#endif

export module model:auth_user;

import std;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

/// @brief A stored auth user — the row the auth app's `db_query` worker persists and reads back
/// through the injected connector. Minimal on purpose: `username` is the primary key, plus the
/// derived `password_hash`.
class AuthUser {
  public:
    /// @brief Default ctor — empty username/password_hash.
    AuthUser() = default;

    /// @brief Sets the username. @param username the new username — also the primary key.
    void set_username(std::string username) { m_username = std::move(username); }
    /// @brief Sets the derived password hash. @param password_hash the new hash.
    void set_password_hash(std::string password_hash) {
        m_password_hash = std::move(password_hash);
    }

    /// @brief Gets the username. @return the username.
    [[nodiscard]] const std::string &get_username() const noexcept { return m_username; }
    /// @brief Gets the derived password hash. @return the hash.
    [[nodiscard]] const std::string &get_password_hash() const noexcept { return m_password_hash; }

  private:
    std::string m_username;
    std::string m_password_hash;
};

} // namespace model

template <>
struct serde::Serializable<model::AuthUser> {
    /// @brief The DB table this user gets persisted to. @return the table name, "auth_users".
    static constexpr std::string_view table_name() { return "auth_users"; }
    /**
     * @brief Field-descriptor table wiring AuthUser's columns to their getters/setters — username
     * is the PK.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"username", &model::AuthUser::get_username,
                         &model::AuthUser::set_username,
                         serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"password_hash", &model::AuthUser::get_password_hash,
                       &model::AuthUser::set_password_hash>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"AuthUser"> auth_user_suite = [] {
    "defaults to empty username/password_hash"_test = [] {
        AuthUser user;

        expect(user.get_username().empty());
        expect(user.get_password_hash().empty());
    };
    "setters round-trip through their getters"_test = [] {
        AuthUser user;
        user.set_username("alice");
        user.set_password_hash("$2b$hash");

        expect(user.get_username() == "alice");
        expect(user.get_password_hash() == "$2b$hash");
    };
};

} // namespace model::tests
#endif

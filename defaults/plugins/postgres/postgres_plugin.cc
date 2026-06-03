module;

#include <congelado/plugin.h>
#include <libpq-fe.h>

export module postgres_plugin;

import congelado_plugin;
import interfaces;
import shared;
import std;

class PostgresPlugin : public congelado::Plugin, public interfaces::IDatabase {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "postgres"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_STORAGE;
    }

    void on_load(congelado::HostCallbacks const &host, congelado::ConfigView const &cfg) override {
        const auto user = std::string{cfg.get("user").value_or("postgres")};
        const auto pass = std::string{cfg.get("password").value_or("")};
        const auto host_s = std::string{cfg.get("host").value_or("localhost")};
        const auto dbname = std::string{cfg.get("dbname").value_or("congelado")};
        const auto port_s = std::string{cfg.get("port").value_or("5432")};

        const auto connstr = std::format("host='{}' port='{}' dbname='{}' user='{}' password='{}'",
                                         host_s, port_s, dbname, user, pass);
        m_conn = PQconnectdb(connstr.c_str());
        if (PQstatus(m_conn) != CONNECTION_OK) {
            host.log(3, std::format("postgres: {}", PQerrorMessage(m_conn)));
            PQfinish(m_conn);
            m_conn = nullptr;
        } else {
            host.log(2, "postgres plugin loaded");
        }
    }

    void on_unload() override {
        if (m_conn) {
            PQfinish(m_conn);
            m_conn = nullptr;
        }
    }

    void *storage_get() noexcept override { return static_cast<interfaces::IDatabase *>(this); }

    [[nodiscard]] std::string_view backend_name() const noexcept override { return "postgres"; }
    [[nodiscard]] bool required() const noexcept override { return true; }

    void query(std::string_view sql, shared::QueryReadFn &&cb) noexcept override {
        exec(sql, std::move(cb));
    }
    void insert(std::string_view sql, shared::QueryReadFn &&cb) noexcept override {
        exec(sql, std::move(cb));
    }
    void update(std::string_view sql, shared::QueryReadFn &&cb) noexcept override {
        exec(sql, std::move(cb));
    }
    void remove(std::string_view sql, shared::QueryReadFn &&cb) noexcept override {
        exec(sql, std::move(cb));
    }

  private:
    PGconn *m_conn{nullptr};

    void exec(std::string_view sql, shared::QueryReadFn cb) noexcept {
        if (!m_conn) {
            cb("");
            return;
        }
        try {
            std::string s{sql};
            PGresult *r = PQexec(m_conn, s.c_str());
            auto st = PQresultStatus(r);
            PQclear(r);
            cb((st == PGRES_COMMAND_OK || st == PGRES_TUPLES_OK) ? "ok" : "");
        } catch (...) {
            cb("");
        }
    }
};

CONGELADO_PLUGIN(PostgresPlugin);

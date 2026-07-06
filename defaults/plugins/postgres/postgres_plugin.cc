module;

#define CONGELADO_GUEST
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

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) override {
        const auto user = std::string{congelado::config_get(cfg, "user").value_or("postgres")};
        const auto pass = std::string{congelado::config_get(cfg, "password").value_or("")};
        const auto host_s = std::string{congelado::config_get(cfg, "host").value_or("localhost")};
        const auto dbname = std::string{congelado::config_get(cfg, "dbname").value_or("congelado")};
        const auto port_s = std::string{congelado::config_get(cfg, "port").value_or("5432")};

        const auto connstr = std::format("host='{}' port='{}' dbname='{}' user='{}' password='{}'",
                                         host_s, port_s, dbname, user, pass);
        m_conn = PQconnectdb(connstr.c_str());
        if (PQstatus(m_conn) != CONNECTION_OK) {
            auto msg = std::format("postgres: {}", PQerrorMessage(m_conn));
            if (host.log) host.log(host.ctx, 3, msg.data(), msg.size());
            PQfinish(m_conn);
            m_conn = nullptr;
        } else {
            if (host.log) host.log(host.ctx, 2, "postgres plugin loaded", 22);
        }
    }

    void on_unload() noexcept override {
        if (m_conn) {
            PQfinish(m_conn);
            m_conn = nullptr;
        }
    }

    void *storage_get() noexcept { return static_cast<interfaces::IDatabase *>(this); }

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

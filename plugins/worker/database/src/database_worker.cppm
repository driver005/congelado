module;

#ifdef CONGELADO_TEST
#    include <rfl/Generic.hpp>
#    include <rfl/json.hpp>
#endif

export module database_worker;

import std;
import interfaces;
import shared;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_db {

/// @brief Typed input for the `database` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<DatabaseInput>` specialization below.
class DatabaseInput
{
public:
    void setSql(std::string value)
    {
        m_sql = std::move(value);
    }

    [[nodiscard]] const std::string& getSql() const noexcept
    {
        return m_sql;
    }

private:
    std::string m_sql;
};

} // namespace worker_db

template<>
struct serde::Serializable<worker_db::DatabaseInput>
{
    static constexpr auto fields()
    {
        using worker_db::DatabaseInput;
        return std::tuple{
            serde::FieldDesc<"sql", &DatabaseInput::getSql, &DatabaseInput::setSql>{},
        };
    }
};

export namespace worker_db {

/// @brief A generic "a database call is a worker" primitive — runs a raw query against the
/// injected `IDatabase` backend (resolved by the worker host, same as the engine's) and returns
/// its result. Shared/reusable (lives in plugins/worker/, not inside any app). Reads `sql` from
/// the input map and returns the backend's result string under `result`. `IDatabase::query` is
/// already callback-based — no thread ever blocks.
class DatabaseWorker final : public interfaces::IWorker
{
public:
    /// @brief Stashes the host-injected `interfaces::IDatabase*` (as an opaque pointer).
    /// @param database_ctx the resolved database backend.
    void set_database_ctx(void* database_ctx) noexcept
    {
        m_database_ctx = database_ctx;
    }

    [[nodiscard]] std::string_view get_task_type() const noexcept override
    {
        return "database";
    }

    void run(const serde::Value& input, interfaces::WorkerCompletion on_complete) override
    {
        auto* database = static_cast<interfaces::IDatabase*>(m_database_ctx);
        if (database == nullptr) {
            on_complete(std::unexpected{interfaces::WorkerError{"no database backend resolved"}});
            return;
        }
        auto parsed = serde::Ser::from_value<DatabaseInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }

        // SECURITY: `sql` is the raw, complete query text taken verbatim from the task's input
        // — set via the zero-auth workflow/task API (see
        // plugins/manager/engine/src/routes.cppm's top-of-file SECURITY note on why "zero-auth"
        // matters here). There is no allowlist, no statement-type restriction (SELECT-only,
        // say), no parameter binding, nothing: any task submitter can run arbitrary SQL —
        // including DDL/DML — against whatever backend the host wired up. This worker is
        // arbitrary-query-execution by design, not a bug in the usual sense, but it's the
        // sharpest edge in this whole file and worth flagging loudly for anyone wiring
        // `database` tasks into a workflow reachable by untrusted input.
        auto shared_complete =
            std::make_shared<interfaces::WorkerCompletion>(std::move(on_complete));
        database->query(parsed->getSql(), [shared_complete](std::string_view rows) {
            (*shared_complete)(
                interfaces::WorkerOutput{{"db_status", "ok"}, {"result", std::string{rows}}}
            );
        });
    }

private:
    void* m_database_ctx{nullptr};
};

} // namespace worker_db

// Unlike client/client_pool/email/llm, this worker's whole side effect goes through a clean
// injectable seam — `interfaces::IDatabase*` — so the full run() surface (success AND failure) is
// testable with a synchronous fake, no real DB or network needed at all.
#ifdef CONGELADO_TEST
namespace worker_db::database_worker_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json)
{
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

/// @brief A synchronous fake `IDatabase` — `query()` fires its callback immediately with a
/// canned result and records the payload it was handed, so tests can assert exactly what SQL
/// text reached the backend without any real connection.
class DatabaseWorkerFakeDatabase final : public interfaces::IDatabase
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake";
    }

    void query(std::string_view payload, shared::QueryReadFn&& result) noexcept override
    {
        m_last_query = std::string{payload};
        ++m_query_count;
        result(m_canned_result);
    }

    void insert(std::string_view, shared::QueryReadFn&& result) noexcept override
    {
        result("");
    }

    void update(std::string_view, shared::QueryReadFn&& result) noexcept override
    {
        result("");
    }

    void remove(std::string_view, shared::QueryReadFn&& result) noexcept override
    {
        result("");
    }

    std::string m_last_query;
    std::string m_canned_result{"row1,row2"};
    int m_query_count{0};
};

suite<"DatabaseInput"> database_input_suite = [] {
    "setSql/getSql round-trips"_test = [] {
        DatabaseInput input;
        input.setSql("SELECT 1");
        expect(input.getSql() == "SELECT 1");
    };

    "default-constructed sql is empty"_test = [] {
        DatabaseInput input;
        expect(input.getSql().empty());
    };

    "from_value fails when 'sql' is omitted"_test = [] {
        auto value = make_value(R"({})");
        auto parsed = serde::Ser::from_value<DatabaseInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("sql")) << parsed.error();
    };

    "from_value succeeds when 'sql' is present"_test = [] {
        auto value = make_value(R"({"sql":"SELECT 1"})");
        auto parsed = serde::Ser::from_value<DatabaseInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getSql() == "SELECT 1");
    };
};

suite<"DatabaseWorker"> database_worker_suite = [] {
    "get_task_type reports 'database'"_test = [] {
        DatabaseWorker worker;
        expect(worker.get_task_type() == "database");
    };

    "run() with no database backend resolved (nullptr default) fails cleanly"_test = [] {
        DatabaseWorker worker;
        auto value = make_value(R"({"sql":"SELECT 1"})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};
        bool called = false;

        worker.run(value, [&](interfaces::WorkerResult result) {
            called = true;
            observed = std::move(result);
        });

        expect(called) << fatal;
        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "no database backend resolved");
    };

    "run() with set_database_ctx(nullptr) behaves identically to never wiring one"_test = [] {
        DatabaseWorker worker;
        worker.set_database_ctx(nullptr);
        auto value = make_value(R"({"sql":"SELECT 1"})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};

        worker.run(value, [&](interfaces::WorkerResult result) {
            observed = std::move(result);
        });

        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "no database backend resolved");
    };

    "run() with a resolved backend runs the query and returns its result verbatim"_test = [] {
        DatabaseWorker worker;
        DatabaseWorkerFakeDatabase database;
        database.m_canned_result = "alice,bob";
        worker.set_database_ctx(&database);

        auto value = make_value(R"({"sql":"SELECT name FROM users"})");
        interfaces::WorkerResult observed = std::unexpected{interfaces::WorkerError{"unset"}};

        worker.run(value, [&](interfaces::WorkerResult result) {
            observed = std::move(result);
        });

        expect(observed.has_value()) << fatal;
        expect(observed->at("db_status") == "ok");
        expect(observed->at("result") == "alice,bob");
        expect(database.m_query_count == 1);
        expect(database.m_last_query == "SELECT name FROM users");
    };

    // SECURITY pin: the worker forwards ANY sql text verbatim, with zero inspection — pins the
    // finding documented above DatabaseWorker::run(). A destructive statement goes through
    // exactly like a SELECT would.
    "SECURITY: run() forwards a destructive statement to the backend with no restriction"_test =
        [] {
            DatabaseWorker worker;
            DatabaseWorkerFakeDatabase database;
            worker.set_database_ctx(&database);

            auto value = make_value(R"({"sql":"DROP TABLE users; --"})");
            interfaces::WorkerResult observed = std::unexpected{interfaces::WorkerError{"unset"}};

            worker.run(value, [&](interfaces::WorkerResult result) {
                observed = std::move(result);
            });

            expect(observed.has_value()) << fatal;
            expect(database.m_last_query == "DROP TABLE users; --");
        };

    "run() with input that doesn't decode into DatabaseInput reports the parse error, never touches the backend"_test =
        [] {
            DatabaseWorker worker;
            DatabaseWorkerFakeDatabase database;
            worker.set_database_ctx(&database);

            auto value = make_value(R"({})"); // missing 'sql'
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};

            worker.run(value, [&](interfaces::WorkerResult result) {
                observed = std::move(result);
            });

            expect(!observed.has_value()) << fatal;
            expect(database.m_query_count == 0);
        };

    "run() with an empty sql string still reaches the backend — no empty-query guard"_test = [] {
        DatabaseWorker worker;
        DatabaseWorkerFakeDatabase database;
        worker.set_database_ctx(&database);

        auto value = make_value(R"({"sql":""})");
        interfaces::WorkerResult observed = std::unexpected{interfaces::WorkerError{"unset"}};

        worker.run(value, [&](interfaces::WorkerResult result) {
            observed = std::move(result);
        });

        expect(observed.has_value()) << fatal;
        expect(database.m_query_count == 1);
        expect(database.m_last_query.empty());
    };
};

} // namespace worker_db::database_worker_tests
#endif

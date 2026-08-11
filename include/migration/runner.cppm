module;

#include <rfl/json.hpp>

export module migration;

import std;

import interfaces;
import connector;
import core_logger;
import core_events;
import utils_hash;

export namespace migration {

using BaselineFn = std::move_only_function<void(
    interfaces::IDatabase &, connector::Connector &, std::move_only_function<void(bool)>)>;

/**
 * @brief Process-wide "has the one global migration pass finished" signal.
 *
 * Set once by whoever drives the host-owned migration run (`congelado::heart::App::load_plugins`,
 * after `migration::Runner::run_all_blocking` returns — success, failure, or a skip because no
 * database is attached/connected all count as "as done as it's going to get"). Anything that
 * can't safely touch the database until its own tables are guaranteed to exist — e.g. a plugin's
 * background sweep thread, started during its own `on_load` before the global migration pass has
 * even run — should poll this instead of guessing at a fixed startup delay.
 */
class Status {
  public:
    /// @brief Marks the global migration pass as finished (successfully or otherwise settled).
    static void mark_ready() noexcept { s_ready.store(true, std::memory_order_release); }
    /// @brief Checks whether the global migration pass has finished.
    /// @return true once mark_ready() has been called.
    [[nodiscard]] static bool is_ready() noexcept { return s_ready.load(std::memory_order_acquire); }

  private:
    static inline std::atomic<bool> s_ready{false};
};

/**
 * @brief Singleton registry where plugins register their baseline migrations.
 *
 * Baselines are meant to create the initial schema for a plugin's models. Each baseline has a
 * name and runs at most once; the runner records `baseline-<name>` in `schema_migrations` after
 * success.
 */
class Registry {
  public:
    /**
     * @brief Process-wide registry instance.
     * @return reference to the singleton.
     */
    [[nodiscard]] static Registry &instance() noexcept {
        static Registry registry;
        return registry;
    }

    /**
     * @brief Registers a baseline migration.
     * @param name unique baseline name, used to build the recorded version `baseline-<name>`.
     * @param fn callback that performs the baseline DDL.
     */
    void add_baseline(std::string name, BaselineFn fn) {
        m_baselines.emplace_back(std::move(name), std::move(fn));
    }

    /**
     * @brief View of registered baselines, in registration order.
     * @return the baseline list.
     */
    [[nodiscard]] std::vector<std::pair<std::string, BaselineFn>> &baselines() noexcept {
        return m_baselines;
    }

    /// @brief Clears all registrations. Mostly useful for tests.
    void clear() noexcept { m_baselines.clear(); }

  private:
    Registry() = default;

    std::vector<std::pair<std::string, BaselineFn>> m_baselines;
};

/**
 * @brief Runs registered baselines and SQL-file migrations on startup.
 */
class Runner {
  public:
    using DoneFn = std::move_only_function<void(bool)>;
    using AppliedMap = std::unordered_map<std::string, std::string>;

    /**
     * @brief Runs all pending migrations asynchronously.
     * @param db the resolved database backend, or nullptr to skip migrations.
     * @param connector the shared connector used by baseline callbacks.
     * @param migrations_dir directory scanned for `<timestamp>_description.sql` migration files.
     * @param done called with `true` if everything applied cleanly.
     */
    static void run_all(interfaces::IDatabase *db, connector::Connector *connector,
                        std::string_view migrations_dir, DoneFn done) {
        if (db == nullptr || connector == nullptr) {
            core::logger::info("migration", "no database configured, skipping migrations");
            done(true);
            return;
        }
        if (!db->is_connected()) {
            core::logger::warning("migration",
                                  "database resolved but not connected, skipping migrations");
            done(true);
            return;
        }

        ensure_schema_migrations(*db, [db, connector, dir = std::string{migrations_dir},
                                       done = std::move(done)](bool ok) mutable {
            if (!ok) {
                core::logger::error("migration", "failed to ensure schema_migrations table");
                done(false);
                return;
            }

            load_applied(*db, [db, connector, dir = std::move(dir),
                               done = std::move(done)](AppliedMap applied) mutable {
                // NOTE: the completion lambda below captures `applied` by copy, not move — a
                // capture-init like `applied = std::move(applied)` runs while this lambda
                // argument is being CONSTRUCTED, i.e. before run_baselines()'s body (which needs
                // `applied` by reference, via the 4th argument) ever executes. Moving here would
                // leave run_baselines() reading an already-emptied map for its whole run.
                run_baselines(*db, *connector, Registry::instance().baselines(), applied,
                              [db, connector, dir = std::move(dir), applied,
                               done = std::move(done)](bool ok) mutable {
                                  if (!ok) {
                                      done(false);
                                      return;
                                  }
                                  run_sql_files(*db, dir, std::move(applied), std::move(done));
                              });
            });
        });
    }

    /**
     * @brief Blocking wrapper around run_all().
     * @return true if all migrations applied cleanly.
     */
    [[nodiscard]] static bool run_all_blocking(interfaces::IDatabase *db,
                                               connector::Connector *connector,
                                               std::string_view migrations_dir) {
        std::promise<bool> promise;
        run_all(db, connector, migrations_dir,
                [&promise](bool ok) { promise.set_value(ok); });
        return promise.get_future().get();
    }

  private:
    struct VersionRow {
        std::string version;
        std::string checksum;
    };

    struct MigrationFile {
        std::string version;
        std::filesystem::path path;
        std::string contents;
        std::string checksum;
    };

    static void ensure_schema_migrations(interfaces::IDatabase &db, DoneFn done) {
        db.query("CREATE TABLE IF NOT EXISTS schema_migrations ("
                 "version TEXT PRIMARY KEY, "
                 "checksum TEXT NOT NULL DEFAULT '', "
                 "applied_at TIMESTAMPTZ NOT NULL DEFAULT now())",
                 [done = std::move(done)](std::string_view result) mutable { done(!result.empty()); });
    }

    static void load_applied(interfaces::IDatabase &db,
                             std::move_only_function<void(AppliedMap)> done) {
        db.query("SELECT version, checksum FROM schema_migrations ORDER BY version",
                 [done = std::move(done)](std::string_view result) mutable {
                     AppliedMap applied;
                     if (!result.empty()) {
                         auto rows = rfl::json::read<std::vector<VersionRow>>(std::string{result});
                         if (rows) {
                             for (auto &row : *rows) {
                                 applied.emplace(std::move(row.version), std::move(row.checksum));
                             }
                         } else {
                             core::logger::warning("migration",
                                                   "failed to parse applied versions: {}",
                                                   rows.error().what());
                         }
                     }
                     done(std::move(applied));
                 });
    }

    static void record_applied(interfaces::IDatabase &db, std::string_view version,
                               std::string_view checksum, DoneFn done) {
        auto sql = std::format("INSERT INTO schema_migrations (version, checksum) VALUES ('{}', '{}')",
                               version, checksum);
        db.query(sql, [done = std::move(done)](std::string_view result) mutable { done(!result.empty()); });
    }

    static void run_baselines(interfaces::IDatabase &db, connector::Connector &connector,
                              std::vector<std::pair<std::string, BaselineFn>> &baselines,
                              AppliedMap &applied, DoneFn done) {
        auto step = std::make_shared<std::function<void(std::size_t)>>();
        auto done_ptr = std::make_shared<DoneFn>(std::move(done));
        // Capture weak in the stored callable to break the self-owning cycle; async callbacks
        // hold a strong `self` so the chain survives across the async gap.
        std::weak_ptr<std::function<void(std::size_t)>> weak_step = step;

        *step = [&db, &connector, &baselines, &applied, weak_step,
                 done_ptr](std::size_t index) mutable {
            auto self = weak_step.lock();
            if (!self) {
                return;
            }
            if (index == baselines.size()) {
                (*done_ptr)(true);
                return;
            }

            auto &[name, fn] = baselines[index];
            auto version = std::format("baseline-{}", name);
            if (applied.contains(version)) {
                (*self)(index + 1);
                return;
            }

            core::logger::info("migration", "applying baseline {}", name);
            fn(db, connector, [self, index, done_ptr, &db, version](bool ok) mutable {
                if (!ok) {
                    core::logger::error("migration", "baseline {} failed", version);
                    (*done_ptr)(false);
                    return;
                }
                record_applied(db, version, "", [self, index, done_ptr](bool ok) mutable {
                    if (!ok) {
                        core::logger::error("migration",
                                            "failed to record applied migration");
                        (*done_ptr)(false);
                        return;
                    }
                    (*self)(index + 1);
                });
            });
        };

        (*step)(0);
    }

    /**
     * @brief Scans `dir` for `<14-digit-timestamp>_description.sql` migration files.
     *
     * Reads each file's contents up front (rather than deferring to run time) so its checksum
     * can be computed once here and reused both for the applied/changed check and for the SQL
     * text actually executed — avoids reading the same file twice.
     */
    [[nodiscard]] static std::vector<MigrationFile> scan_migrations(const std::filesystem::path &dir) {
        std::vector<MigrationFile> files;
        if (!std::filesystem::exists(dir)) {
            return files;
        }

        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const auto filename = entry.path().filename().string();
            const auto underscore = filename.find('_');
            const auto dot = filename.rfind('.');
            if (underscore == std::string::npos || dot == std::string::npos || dot <= underscore) {
                continue;
            }
            if (filename.substr(dot) != ".sql") {
                continue;
            }

            const auto timestamp = filename.substr(0, underscore);
            constexpr std::size_t timestamp_width = 14; // YYYYMMDDHHMMSS
            if (timestamp.size() != timestamp_width ||
                !std::all_of(timestamp.begin(), timestamp.end(),
                             [](unsigned char c) { return std::isdigit(c); })) {
                core::logger::warning("migration",
                                      "ignoring malformed migration file (expected "
                                      "<YYYYMMDDHHMMSS>_description.sql): {}",
                                      filename);
                continue;
            }

            std::ifstream stream(entry.path());
            if (!stream) {
                core::logger::warning("migration", "cannot open migration file: {}", filename);
                continue;
            }
            std::string contents((std::istreambuf_iterator<char>(stream)),
                                 std::istreambuf_iterator<char>());
            if (contents.empty()) {
                core::logger::warning("migration", "migration file is empty: {}", filename);
                continue;
            }

            auto checksum = utils::Sha256::hash_hex(contents);
            files.push_back(MigrationFile{.version = filename.substr(0, dot),
                                          .path = entry.path(),
                                          .contents = std::move(contents),
                                          .checksum = std::move(checksum)});
        }

        std::sort(files.begin(), files.end(), [](const MigrationFile &a, const MigrationFile &b) {
            return a.version < b.version;
        });

        return files;
    }

    static void run_sql_file(interfaces::IDatabase &db, const MigrationFile &file, DoneFn done) {
        auto sql = std::format("BEGIN; {} COMMIT;", file.contents);
        core::logger::info("migration", "applying migration {}", file.version);
        db.query(sql, [&db, file, done = std::move(done)](std::string_view result) mutable {
            if (result.empty()) {
                core::logger::error("migration", "migration {} failed", file.version);
                done(false);
                return;
            }
            record_applied(db, file.version, file.checksum, std::move(done));
        });
    }

    /**
     * @brief Filters scanned migration files against `applied` and runs whatever's pending.
     *
     * Three outcomes per file: never applied (run it), applied with a matching (or unknown/
     * empty) stored checksum (skip, already run), or applied with a checksum that no longer
     * matches the file on disk (hard-fail — migrations are immutable once applied; a content
     * change after the fact means either accidental drift or an edit that should have been a
     * new versioned file instead).
     */
    static void run_sql_files(interfaces::IDatabase &db, const std::filesystem::path &dir,
                              AppliedMap applied, DoneFn done) {
        auto files = scan_migrations(dir);
        std::vector<MigrationFile> pending;
        for (auto &file : files) {
            auto it = applied.find(file.version);
            if (it == applied.end()) {
                pending.push_back(std::move(file));
                continue;
            }

            const auto &stored_checksum = it->second;
            if (stored_checksum.empty() || stored_checksum == file.checksum) {
                continue;
            }

            core::logger::error("migration",
                                "migration {} was already applied with checksum {} but the file "
                                "on disk now hashes to {} — refusing to continue; migrations must "
                                "be immutable once applied, add a new versioned file for further "
                                "changes instead of editing an applied one",
                                file.version, stored_checksum, file.checksum);
            done(false);
            return;
        }

        if (pending.empty()) {
            done(true);
            return;
        }

        auto step = std::make_shared<std::function<void(std::size_t)>>();
        auto done_ptr = std::make_shared<DoneFn>(std::move(done));
        // Capture weak in the stored callable to break the self-owning cycle; async callbacks
        // hold a strong `self` so the chain survives across the async gap.
        std::weak_ptr<std::function<void(std::size_t)>> weak_step = step;

        *step = [&db, pending = std::move(pending), weak_step, done_ptr](std::size_t index) mutable {
            auto self = weak_step.lock();
            if (!self) {
                return;
            }
            if (index == pending.size()) {
                (*done_ptr)(true);
                return;
            }
            run_sql_file(db, pending[index], [self, index, done_ptr](bool ok) mutable {
                if (!ok) {
                    (*done_ptr)(false);
                    return;
                }
                (*self)(index + 1);
            });
        };

        (*step)(0);
    }
};

} // namespace migration

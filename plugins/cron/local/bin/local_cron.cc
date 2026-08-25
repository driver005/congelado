module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module local_cron_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_contract;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief A hand-rolled 5-field cron expression (`minute hour day-of-month month day-of-week`, the
/// same shape as any standard crontab line). Supports `*`, a bare number, `N-M` ranges, `*/N`
/// steps, and comma-separated lists of any of those, in each field — covers the large majority of
/// real-world cron expressions without a full grammar. This is the "local" cron engine: it used to
/// live inside the engine plugin and now backs the default `ICron` implementation.
class CronExpression {
  public:
    /**
     * @brief Parses a 5-field cron expression.
     * @param expr the cron string, e.g. `"*\/5 * * * *"` (every 5 minutes) or `"0 9 * * 1-5"`
     * (9am on weekdays).
     * @return the parsed expression, or std::nullopt if it doesn't have exactly 5
     * whitespace-separated fields or any field fails to parse.
     */
    [[nodiscard]] static std::optional<CronExpression> parse(std::string_view expr) {
        std::vector<std::string_view> fields;
        std::size_t start = 0;
        while (start < expr.size()) {
            while (start < expr.size() && expr[start] == ' ') {
                ++start;
            }
            if (start >= expr.size()) {
                break;
            }
            auto end = expr.find(' ', start);
            if (end == std::string_view::npos) {
                end = expr.size();
            }
            fields.push_back(expr.substr(start, end - start));
            start = end;
        }
        if (fields.size() != 5) {
            return std::nullopt;
        }

        CronExpression result;
        auto minutes = parse_field(fields[0], 0, 59);
        auto hours = parse_field(fields[1], 0, 23);
        auto days = parse_field(fields[2], 1, 31);
        auto months = parse_field(fields[3], 1, 12);
        auto weekdays = parse_field(fields[4], 0, 6);
        if (!minutes || !hours || !days || !months || !weekdays) {
            return std::nullopt;
        }
        result.m_minutes = std::move(*minutes);
        result.m_hours = std::move(*hours);
        result.m_days = std::move(*days);
        result.m_months = std::move(*months);
        result.m_weekdays = std::move(*weekdays);
        return result;
    }

    /**
     * @brief Finds the next time (strictly after `base`) that every field matches.
     * @warning Brute-force, minute by minute — fine for an infrequent (every-few-seconds tick)
     * calculation, not something to call in a hot loop. Gives up and returns std::nullopt after
     * scanning 4 years forward (covers even a lone Feb-29-only expression) rather than spinning
     * forever on an expression that can never actually match.
     * @param base the time to search strictly after.
     * @return the next matching minute-aligned time_point, or std::nullopt if none was found within
     * the search horizon.
     */
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point>
    next_after(std::chrono::system_clock::time_point base) const {
        using namespace std::chrono;
        auto candidate = floor<minutes>(base) + minutes{1};
        constexpr auto horizon = minutes{4 * 366 * 24 * 60};
        auto deadline = candidate + horizon;
        while (candidate < deadline) {
            if (matches(candidate)) {
                return candidate;
            }
            candidate += minutes{1};
        }
        return std::nullopt;
    }

  private:
    std::vector<int> m_minutes;
    std::vector<int> m_hours;
    std::vector<int> m_days;
    std::vector<int> m_months;
    std::vector<int> m_weekdays;

    [[nodiscard]] bool matches(std::chrono::system_clock::time_point candidate) const {
        using namespace std::chrono;
        auto days_point = floor<days>(candidate);
        year_month_day ymd{days_point};
        auto time_of_day = hh_mm_ss{candidate - days_point};
        weekday wd{days_point};

        auto contains = [](std::vector<int> const &values, int value) {
            return std::ranges::find(values, value) != values.end();
        };
        return contains(m_minutes, static_cast<int>(time_of_day.minutes().count())) &&
               contains(m_hours, static_cast<int>(time_of_day.hours().count())) &&
               contains(m_days, static_cast<int>(static_cast<unsigned>(ymd.day()))) &&
               contains(m_months, static_cast<int>(static_cast<unsigned>(ymd.month()))) &&
               contains(m_weekdays, static_cast<int>(wd.c_encoding()));
    }

    /// @brief Parses one cron field into the explicit set of values it matches — `*` expands to the
    /// whole `[min, max]` range, `*/N` to every Nth value in that range, `A-B` to a range, a bare
    /// number to itself, and a comma joins any mix of the above.
    [[nodiscard]] static std::optional<std::vector<int>> parse_field(std::string_view field,
                                                                     int min, int max) {
        std::vector<int> values;
        std::size_t start = 0;
        while (start <= field.size()) {
            auto comma = field.find(',', start);
            auto token = comma == std::string_view::npos ? field.substr(start)
                                                         : field.substr(start, comma - start);
            if (!parse_token(token, min, max, values)) {
                return std::nullopt;
            }
            if (comma == std::string_view::npos) {
                break;
            }
            start = comma + 1;
        }
        if (values.empty()) {
            return std::nullopt;
        }
        std::ranges::sort(values);
        values.erase(std::ranges::unique(values).begin(), values.end());
        return values;
    }

    [[nodiscard]] static bool parse_token(std::string_view token, int min, int max,
                                          std::vector<int> &out) {
        if (token.empty()) {
            return false;
        }
        if (token == "*") {
            for (int value = min; value <= max; ++value) {
                out.push_back(value);
            }
            return true;
        }
        if (token.starts_with("*/")) {
            int step = 0;
            if (!parse_int(token.substr(2), step) || step <= 0) {
                return false;
            }
            for (int value = min; value <= max; value += step) {
                out.push_back(value);
            }
            return true;
        }
        if (auto dash = token.find('-'); dash != std::string_view::npos) {
            int lo = 0;
            int hi = 0;
            if (!parse_int(token.substr(0, dash), lo) || !parse_int(token.substr(dash + 1), hi) ||
                lo > hi) {
                return false;
            }
            for (int value = std::max(lo, min); value <= std::min(hi, max); ++value) {
                out.push_back(value);
            }
            return true;
        }
        int value = 0;
        if (!parse_int(token, value) || value < min || value > max) {
            return false;
        }
        out.push_back(value);
        return true;
    }

    [[nodiscard]] static bool parse_int(std::string_view text, int &out) {
        auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        return result.ec == std::errc{} && result.ptr == text.data() + text.size();
    }
};

/// @brief The default, in-process `ICron` backend: a poll-sweep timing engine. Holds a registry of
/// named jobs (each a parsed CronExpression plus its last fire time) and, on each tick, fires the
/// callback for every job whose next occurrence has passed. The single mutex guards the job map and
/// the fire callback against the tick thread racing request-handling threads that upsert/remove
/// jobs — cron runs on the shared contract pool, genuinely concurrent with those callers, so this
/// is one of the few spots that actually needs a lock rather than ASIO serialization.
class LocalCron : public interfaces::ICron {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "local"; }

    [[nodiscard]] bool validate(std::string_view cron_expression) const noexcept override {
        return CronExpression::parse(cron_expression).has_value();
    }

    [[nodiscard]] std::optional<std::chrono::system_clock::time_point>
    next_after(std::string_view cron_expression,
               std::chrono::system_clock::time_point base) const noexcept override {
        auto parsed = CronExpression::parse(cron_expression);
        if (!parsed) {
            return std::nullopt;
        }
        return parsed->next_after(base);
    }

    void set_fire_callback(std::move_only_function<void(std::string_view)> callback) override {
        std::lock_guard lock{m_mutex};
        m_fire = std::move(callback);
    }

    void upsert_job(std::string_view name, std::string_view cron_expression) override {
        auto parsed = CronExpression::parse(cron_expression);
        if (!parsed) {
            core::logger::warning("cron.local", "job '{}' has unparseable cron_expression '{}'",
                                  name, cron_expression);
            return;
        }
        std::lock_guard lock{m_mutex};
        auto found = m_jobs.find(std::string{name});
        if (found != m_jobs.end()) {
            found->second.expr = std::move(*parsed);
            return;
        }
        // A newly tracked job bases its next-fire search one minute in the past, so a schedule
        // registered during the very minute it should fire still fires — same "never fired ⇒ now
        // minus one minute" base the old engine sweep used.
        m_jobs.emplace(std::string{name},
                       Job{std::move(*parsed),
                           std::chrono::system_clock::now() - std::chrono::minutes{1}});
    }

    void remove_job(std::string_view name) override {
        std::lock_guard lock{m_mutex};
        m_jobs.erase(std::string{name});
    }

    /// @brief One sweep of the job registry — fires every job whose next occurrence (after its last
    /// fire) has passed, stamping its last fire time to now. Fires under the lock; the callback only
    /// enqueues async connector/orchestrator work and returns fast, so it never blocks the tick.
    void tick() {
        std::lock_guard lock{m_mutex};
        if (!m_fire) {
            return;
        }
        auto now = std::chrono::system_clock::now();
        for (auto &[name, job] : m_jobs) {
            auto next = job.expr.next_after(job.last_fired);
            if (next && *next <= now) {
                job.last_fired = now;
                m_fire(name);
            }
        }
    }

  private:
    struct Job {
        CronExpression expr;
        std::chrono::system_clock::time_point last_fired;
    };

    std::mutex m_mutex;
    std::unordered_map<std::string, Job> m_jobs;
    std::move_only_function<void(std::string_view)> m_fire;
};

/// @brief The background tick contract driving LocalCron — a `core::contract` handler on the shared
/// pool (same mechanism the engine's own sweep uses), sleeping a second between sweeps and
/// self-rescheduling, rather than a bespoke thread.
class CronTickHandler : public shared::HandlerBase {
  public:
    explicit CronTickHandler(LocalCron &cron) noexcept : m_cron{cron} {}

    [[nodiscard]] std::string_view get_name() const noexcept override { return "cron.local.tick"; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            m_cron.get().tick();
            std::this_thread::sleep_for(std::chrono::seconds{1});
            shared::this_handler::shedule();
        };
    }

  private:
    std::reference_wrapper<LocalCron> m_cron;
};

/// @brief The default cron plugin — exports the CRON capability backed by LocalCron and registers
/// its tick contract on load.
class CronLocalPlugin : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "cron_local"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }

    /**
     * @brief Flags this as cron-capable, so the host wires `cron_get` into the `_cap_dispatch`
     * routing and resolves this plugin's ICron* for the engine.
     * @return `CONGELADO_CAP_CRON`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_CRON;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `ICron` surface.
     * @return this plugin's LocalCron, upcast to `interfaces::ICron*`.
     */
    void *cron_get() noexcept { return static_cast<interfaces::ICron *>(&m_cron); }

    /**
     * @brief Registers the background tick contract on the host's contract pool.
     * @note No contract group/registry, no motion — logs an error and bails rather than
     * dereferencing a null pointer; schedules then never fire.
     * @param host the host callback table; supplies the contract group and registry.
     * @param cfg unused — this plugin reads no config.
     */
    void on_load(CongeladoHostCallbacks const &host,
                 CongeladoConfigView const & /*cfg*/) override {
        auto *contract_group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
        auto *contract_registry = congelado::registry_ctx<core::contract::ContractRegistry>(host);
        if (contract_group == nullptr || contract_registry == nullptr) {
            core::logger::error("cron.local", "no contract group/registry — cron tick not started");
            return;
        }
        contract_registry->add(
            m_tick.create(*contract_group, core::contract::ContractState::SCHEDULED));
        core::logger::important("cron.local", "cron tick started");
    }

  private:
    LocalCron m_cron;
    CronTickHandler m_tick{m_cron};
};

} // namespace

CONGELADO_PLUGIN(CronLocalPlugin);

#ifdef CONGELADO_TEST
namespace local_cron_tests {
using namespace boost::ut;

suite<"CronExpression::parse"> cron_expression_parse_suite = [] {
    "wildcard in every field parses successfully"_test = [] {
        expect(CronExpression::parse("* * * * *").has_value());
    };

    "a specific value in every field parses successfully"_test = [] {
        expect(CronExpression::parse("30 14 15 6 3").has_value());
    };

    "*/N step syntax parses successfully"_test = [] {
        expect(CronExpression::parse("*/15 * * * *").has_value());
    };

    "A-B range syntax parses successfully"_test = [] {
        expect(CronExpression::parse("0 9-17 * * *").has_value());
    };

    "comma-separated list parses successfully"_test = [] {
        expect(CronExpression::parse("0,15,30,45 * * * *").has_value());
    };

    "a list mixing bare values and ranges parses successfully"_test = [] {
        expect(CronExpression::parse("0 9,12-14,18 * * *").has_value());
    };

    "leading/trailing and repeated internal whitespace is tolerated"_test = [] {
        expect(CronExpression::parse("  *   *  * *   * ").has_value());
    };

    "fewer than 5 fields fails"_test = [] {
        expect(!CronExpression::parse("* * * *").has_value());
    };

    "more than 5 fields fails"_test = [] {
        expect(!CronExpression::parse("* * * * * *").has_value());
    };

    "empty string fails"_test = [] { expect(!CronExpression::parse("").has_value()); };

    "an out-of-range minute fails"_test = [] {
        expect(!CronExpression::parse("60 * * * *").has_value());
    };

    "an out-of-range hour fails"_test = [] {
        expect(!CronExpression::parse("* 24 * * *").has_value());
    };

    "an out-of-range (zero) day-of-month fails"_test = [] {
        expect(!CronExpression::parse("* * 0 * *").has_value());
    };

    "an out-of-range month fails"_test = [] {
        expect(!CronExpression::parse("* * * 13 *").has_value());
    };

    "an out-of-range weekday fails"_test = [] {
        expect(!CronExpression::parse("* * * * 7").has_value());
    };

    "a non-numeric token fails"_test = [] {
        expect(!CronExpression::parse("abc * * * *").has_value());
    };

    "an inverted range (lo > hi) fails"_test = [] {
        expect(!CronExpression::parse("30-10 * * * *").has_value());
    };

    "a zero step fails"_test = [] {
        expect(!CronExpression::parse("*/0 * * * *").has_value());
    };

    "an empty token inside a comma list fails"_test = [] {
        expect(!CronExpression::parse("0,, * * * *").has_value());
    };
};

suite<"CronExpression::next_after"> cron_expression_next_after_suite = [] {
    "every-minute expression returns the next whole-minute boundary"_test = [] {
        using namespace std::chrono;
        auto expr = CronExpression::parse("* * * * *");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}} + hours{10} + minutes{30} + seconds{15};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{10}} + hours{10} + minutes{31});
    };

    "comma list fires at the next listed minute"_test = [] {
        using namespace std::chrono;
        auto expr = CronExpression::parse("0,30 * * * *");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}} + hours{10} + minutes{5};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{10}} + hours{10} + minutes{30});
    };

    "*/15 step fires every 15 minutes"_test = [] {
        using namespace std::chrono;
        auto expr = CronExpression::parse("*/15 * * * *");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}} + hours{10} + minutes{16};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{10}} + hours{10} + minutes{30});
    };

    "hour range (9-17) rolls to the next day once past today's window"_test = [] {
        using namespace std::chrono;
        auto expr = CronExpression::parse("0 9-17 * * *");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}} + hours{18};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{11}} + hours{9});
    };

    "specific hour rolls to the next day once today's slot has passed"_test = [] {
        using namespace std::chrono;
        auto expr = CronExpression::parse("0 9 * * *");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}} + hours{10} + minutes{30};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{11}} + hours{9});
    };

    "specific weekday rolls forward to the next matching day"_test = [] {
        using namespace std::chrono;
        // 2024-04-10 is a Wednesday; the expression matches Mondays only (weekday 1).
        auto expr = CronExpression::parse("0 0 * * 1");
        expect(expr.has_value()) << fatal;

        auto base = sys_days{year{2024} / April / day{10}};
        auto result = expr->next_after(base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{15}});
    };

    "day-of-month/month rollover: day 31 in a 30-day month rolls to the next 31-day month"_test =
        [] {
            using namespace std::chrono;
            // April has no 31st; May does, so the next match must skip all of April.
            auto expr = CronExpression::parse("0 0 31 * *");
            expect(expr.has_value()) << fatal;

            auto base = sys_days{year{2024} / April / day{10}};
            auto result = expr->next_after(base);

            expect(result.has_value()) << fatal;
            expect(*result == sys_days{year{2024} / May / day{31}});
        };

    // Slower test by design: Feb 31 never occurs on the real calendar, so this exercises the
    // documented 4-year search horizon giving up and returning std::nullopt rather than looping
    // forever. Brute-forcing ~4 years of minutes is a few million cheap iterations — noticeably
    // slower than the other cases here, but still bounded and deterministic.
    "an expression that can never match (Feb 31) exhausts the horizon and returns nullopt"_test =
        [] {
            using namespace std::chrono;
            auto expr = CronExpression::parse("0 0 31 2 *");
            expect(expr.has_value()) << fatal;

            auto base = sys_days{year{2024} / April / day{10}};
            auto result = expr->next_after(base);

            expect(!result.has_value());
        };
};

class MockHandlerInterface final : public shared::HandlerInterface {
  public:
    void schedule(std::uint32_t) override { ++m_schedule_count; }
    void deschedule(std::uint32_t) override { ++m_deschedule_count; }
    void release(std::uint32_t) override { ++m_release_count; }

    int m_schedule_count{0};
    int m_deschedule_count{0};
    int m_release_count{0};
};

suite<"LocalCron"> local_cron_suite = [] {
    "backend_name reports 'local'"_test = [] {
        LocalCron cron;
        expect(cron.backend_name() == "local");
    };

    "validate reports well-formed vs malformed expressions"_test = [] {
        LocalCron cron;
        expect(cron.validate("* * * * *"));
        expect(!cron.validate("not a cron expression"));
    };

    "next_after mirrors CronExpression::next_after for a valid expression"_test = [] {
        using namespace std::chrono;
        LocalCron cron;
        auto base = sys_days{year{2024} / April / day{10}} + hours{10} + minutes{30};

        auto result = cron.next_after("* * * * *", base);

        expect(result.has_value()) << fatal;
        expect(*result == sys_days{year{2024} / April / day{10}} + hours{10} + minutes{31});
    };

    "next_after returns nullopt for an unparseable expression"_test = [] {
        LocalCron cron;
        auto result = cron.next_after("garbage", std::chrono::system_clock::now());
        expect(!result.has_value());
    };

    "a job upserted with a matching expression fires on tick"_test = [] {
        LocalCron cron;
        std::vector<std::string> fired;
        cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        cron.upsert_job("every-minute", "* * * * *");
        cron.tick();

        expect(fired.size() == 1) << fatal;
        expect(fired[0] == "every-minute");
    };

    "upsert_job with an unparseable expression is silently dropped, never fires"_test = [] {
        LocalCron cron;
        std::vector<std::string> fired;
        cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        cron.upsert_job("bad-job", "not a cron expression");
        cron.tick();

        expect(fired.empty());
    };

    "remove_job stops a previously registered job from firing"_test = [] {
        LocalCron cron;
        std::vector<std::string> fired;
        cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        cron.upsert_job("temp-job", "* * * * *");
        cron.remove_job("temp-job");
        cron.tick();

        expect(fired.empty());
    };

    "tick with no fire callback installed is a safe no-op"_test = [] {
        LocalCron cron;
        cron.upsert_job("orphan", "* * * * *");
        expect(nothrow([&] { cron.tick(); }));
    };

    "re-upserting an existing job's name replaces its expression"_test = [] {
        LocalCron cron;
        std::vector<std::string> fired;
        cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        // First register with an expression that can never fire (Feb 31), then replace it with
        // one that matches immediately — only the replacement should ever fire.
        cron.upsert_job("job", "0 0 31 2 *");
        cron.upsert_job("job", "* * * * *");
        cron.tick();

        expect(fired.size() == 1) << fatal;
        expect(fired[0] == "job");
    };
};

suite<"CronTickHandler"> cron_tick_handler_suite = [] {
    "get_name returns the fixed tick handler name"_test = [] {
        LocalCron cron;
        CronTickHandler handler{cron};
        expect(handler.get_name() == "cron.local.tick");
    };

    // Slower test by design: CronTickHandler::on_execute()'s returned worker sleeps 1 real
    // second between ticking and rescheduling itself (see the class's own doc comment) — that
    // sleep is exercised here rather than mocked out, since it's genuinely part of the behavior
    // under test.
    "on_execute ticks the bound LocalCron and reschedules itself via this_handler"_test = [] {
        LocalCron cron;
        bool fired = false;
        cron.set_fire_callback([&](std::string_view) { fired = true; });
        cron.upsert_job("j", "* * * * *");

        CronTickHandler handler{cron};

        MockHandlerInterface mock;
        shared::this_handler::current = &mock;
        shared::this_handler::current_id = 1;

        auto worker = handler.on_execute();
        worker();

        expect(fired);
        expect(mock.m_schedule_count == 1);

        shared::this_handler::current = nullptr;
    };
};

suite<"CronLocalPlugin"> cron_local_plugin_suite = [] {
    "get_name/get_version/capabilities report the plugin's fixed identity"_test = [] {
        CronLocalPlugin plugin;
        expect(plugin.get_name() == "cron_local");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.capabilities() == CONGELADO_CAP_CRON);
    };

    "cron_get exposes the plugin's LocalCron through the ICron interface"_test = [] {
        CronLocalPlugin plugin;
        auto *cron = static_cast<interfaces::ICron *>(plugin.cron_get());

        expect(cron != nullptr) << fatal;
        expect(cron->backend_name() == "local");
    };

    "on_load with no contract group/registry logs and returns without crashing"_test = [] {
        CronLocalPlugin plugin;
        CongeladoHostCallbacks host{};
        CongeladoConfigView cfg{};

        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };

    "on_load with a valid contract group/registry registers the tick contract"_test = [] {
        CronLocalPlugin plugin;
        core::contract::ContractGroup<> group;
        core::contract::ContractRegistry registry;

        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        host.registry_ctx = &registry;
        CongeladoConfigView cfg{};

        expect(registry.empty());
        plugin.on_load(host, cfg);
        expect(registry.size() == 1);

        registry.release_all();
    };
};

} // namespace local_cron_tests
#endif

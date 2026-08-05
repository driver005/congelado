export module engine:cron;

import std;
import model;
import core_logger;

export namespace engine {

/// @brief A hand-rolled 5-field cron expression (`minute hour day-of-month month day-of-week`,
/// the same shape as any standard crontab line) — nothing cron-shaped existed anywhere in this
/// codebase before, and pulling in a whole vendored cron library for "parse 5 fields and find
/// the next matching minute" felt disproportionate. Supports `*`, a bare number, `N-M` ranges,
/// `*/N` steps, and comma-separated lists of any of those, in each field — covers the large
/// majority of real-world cron expressions without needing a full grammar.
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
     * @warning Brute-force, minute by minute — fine for an infrequent (every-few-seconds sweep
     * tick) calculation, not something to call in a hot loop. Gives up and returns std::nullopt
     * after scanning 4 years forward (covers even a lone Feb-29-only expression) rather than
     * spinning forever on an expression that can never actually match.
     * @param base the time to search strictly after.
     * @return the next matching minute-aligned time_point, or std::nullopt if none was found
     * within the search horizon.
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

    /// @brief Parses one cron field into the explicit set of values it matches — `*` expands to
    /// the whole `[min, max]` range, `*/N` to every Nth value in that range, `A-B` to a range,
    /// a bare number to itself, and a comma joins any mix of the above.
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

} // namespace engine

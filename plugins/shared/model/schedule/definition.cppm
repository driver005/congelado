export module model:schedule_def;

import std;
import :timestamps;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

/// @brief A cron-driven auto-start rule for a WorkflowDef — Conductor's own scheduler
/// equivalent (a dedicated module there; here, driven off the same background sweep the
/// orchestrator already runs, see engine::CronScheduler).
class WorkflowSchedule
{
public:
    WorkflowSchedule() = default;

    void set_name(std::string name)
    {
        m_name = std::move(name);
    }

    void set_workflow_name(std::string workflow_name)
    {
        m_workflow_name = std::move(workflow_name);
    }

    void set_workflow_version(std::uint32_t version) noexcept
    {
        m_workflow_version = version;
    }

    void set_cron_expression(std::string cron_expression)
    {
        m_cron_expression = std::move(cron_expression);
    }

    void set_seed_variables(std::unordered_map<std::string, std::string> variables)
    {
        m_seed_variables = std::move(variables);
    }

    void set_enabled(bool enabled) noexcept
    {
        m_enabled = enabled;
    }

    void set_paused(bool paused) noexcept
    {
        m_paused = paused;
    }

    void set_last_fired_at(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_last_fired_at = value;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_workflow_name() const noexcept
    {
        return m_workflow_name;
    }

    [[nodiscard]] std::uint32_t get_workflow_version() const noexcept
    {
        return m_workflow_version;
    }

    [[nodiscard]] const std::string& get_cron_expression() const noexcept
    {
        return m_cron_expression;
    }

    [[nodiscard]] const std::unordered_map<std::string, std::string>&
    get_seed_variables() const noexcept
    {
        return m_seed_variables;
    }

    [[nodiscard]] bool get_enabled() const noexcept
    {
        return m_enabled;
    }

    [[nodiscard]] bool get_paused() const noexcept
    {
        return m_paused;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_last_fired_at() const noexcept
    {
        return m_last_fired_at;
    }

    /**
     * @brief Checks that name/workflow_name/cron_expression are set — no cap, that's the whole
     * check. Doesn't validate the cron expression actually parses; CronScheduler treats an
     * unparseable one as "never fires" rather than erroring here.
     * @return an empty expected if everything's non-empty, otherwise an unexpected naming
     * whichever one's blank.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept
    {
        if (m_name.empty()) {
            return std::unexpected{"WorkflowSchedule name must not be empty"};
        }
        if (m_workflow_name.empty()) {
            return std::unexpected{"WorkflowSchedule workflow_name must not be empty"};
        }
        if (m_cron_expression.empty()) {
            return std::unexpected{"WorkflowSchedule cron_expression must not be empty"};
        }
        return {};
    }

private:
    std::string m_name;
    std::string m_workflow_name;
    std::uint32_t m_workflow_version{1};
    std::string m_cron_expression;
    std::unordered_map<std::string, std::string> m_seed_variables;
    bool m_enabled{true};
    bool m_paused{false};
    std::optional<std::chrono::system_clock::time_point> m_last_fired_at;
};

} // namespace model

template<>
struct serde::Serializable<model::WorkflowSchedule>
{
    static constexpr std::string_view table_name()
    {
        return "workflow_schedules";
    }

    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name", &model::WorkflowSchedule::get_name, &model::WorkflowSchedule::set_name,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "workflow_name", &model::WorkflowSchedule::get_workflow_name,
                &model::WorkflowSchedule::set_workflow_name>{},
            serde::FieldDesc<
                "workflow_version", &model::WorkflowSchedule::get_workflow_version,
                &model::WorkflowSchedule::set_workflow_version>{},
            serde::FieldDesc<
                "cron_expression", &model::WorkflowSchedule::get_cron_expression,
                &model::WorkflowSchedule::set_cron_expression>{},
            serde::FieldDesc<
                "seed_variables", &model::WorkflowSchedule::get_seed_variables,
                &model::WorkflowSchedule::set_seed_variables>{},
            serde::FieldDesc<
                "enabled", &model::WorkflowSchedule::get_enabled,
                &model::WorkflowSchedule::set_enabled>{},
            serde::FieldDesc<
                "paused", &model::WorkflowSchedule::get_paused,
                &model::WorkflowSchedule::set_paused>{},
            serde::FieldDesc<
                "last_fired_at", &model::WorkflowSchedule::get_last_fired_at,
                &model::WorkflowSchedule::set_last_fired_at>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"WorkflowSchedule"> workflow_schedule_suite = [] {
    "defaults to enabled, not paused, version 1, and fails validation"_test = [] {
        WorkflowSchedule schedule;

        expect(schedule.get_enabled());
        expect(not schedule.get_paused());
        expect(schedule.get_workflow_version() == 1);
        expect(not schedule.validate().has_value());
    };
    "requires name, workflow_name, and cron_expression"_test = [] {
        WorkflowSchedule schedule;
        schedule.set_name("nightly_report");
        schedule.set_workflow_name("report_pipeline");
        expect(not schedule.validate().has_value());

        schedule.set_cron_expression("0 0 * * *");
        expect(bool(schedule.validate()));
    };
    "setters round-trip through their getters"_test = [] {
        WorkflowSchedule schedule;
        schedule.set_seed_variables({{"region", "eu"}});
        schedule.set_enabled(false);
        schedule.set_paused(true);

        expect(schedule.get_seed_variables().at("region") == "eu");
        expect(not schedule.get_enabled());
        expect(schedule.get_paused());
    };
};

} // namespace model::tests
#endif

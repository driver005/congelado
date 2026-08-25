export module model:poll_data;

import std;
import :timestamps;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

/// @brief One worker type's last-seen poll heartbeat — populated by TaskHandler::poll() on every
/// hit (worker_type is whatever the caller polled for, hit or miss). Backs the
/// `GET /api/v1/tasks/queue/polldata` admin introspection route; actual queue depth is computed
/// on demand by `/queue/sizes` rather than cached here, so this is purely "is anything out there
/// polling this type, and when did it last check in."
class PollData {
  public:
    PollData() = default;

    void set_worker_type(std::string worker_type) { m_worker_type = std::move(worker_type); }
    void set_last_poll_at(std::chrono::system_clock::time_point value) noexcept {
        m_last_poll_at = value;
    }

    [[nodiscard]] const std::string &get_worker_type() const noexcept { return m_worker_type; }
    [[nodiscard]] std::chrono::system_clock::time_point get_last_poll_at() const noexcept {
        return m_last_poll_at;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_worker_type.empty()) {
            return std::unexpected{"PollData worker_type must not be empty"};
        }
        return {};
    }

  private:
    std::string m_worker_type;
    std::chrono::system_clock::time_point m_last_poll_at;
};

} // namespace model

template <>
struct serde::Serializable<model::PollData> {
    static constexpr std::string_view table_name() { return "poll_data"; }
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"worker_type", &model::PollData::get_worker_type,
                       &model::PollData::set_worker_type,
                       serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"last_poll_at", &model::PollData::get_last_poll_at,
                       &model::PollData::set_last_poll_at>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"PollData"> poll_data_suite = [] {
    "defaults to an empty worker_type and fails validation"_test = [] {
        PollData data;
        expect(not data.validate().has_value());
    };
    "setters round-trip and a non-empty worker_type passes validation"_test = [] {
        PollData data;
        auto now = std::chrono::system_clock::now();
        data.set_worker_type("email_worker");
        data.set_last_poll_at(now);

        expect(data.get_worker_type() == "email_worker");
        expect(data.get_last_poll_at() == now);
        expect(bool(data.validate()));
    };
};

} // namespace model::tests
#endif

export module io_layer_shared:ping;

import std;

export namespace transport::shared_layer::ping {

class PingTracker {
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using Payload = std::array<std::uint8_t, 8>;

    struct RttStats {
        Duration srtt{};   ///< smoothed RTT (EWMA, α=0.125)
        Duration rttvar{}; ///< RTT variance (EWMA, β=0.25)
        Duration min_rtt{Duration::max()};
        Duration max_rtt{};
        std::size_t samples{0};
    };

    enum class ActionKind { SendPing, SendGoaway };

    struct Action {
        ActionKind kind;
        Payload payload{}; ///< valid when kind == SendPing
    };

    struct Config {
        /// How long to wait before sending a keepalive PING when idle.
        Duration keepalive_interval = std::chrono::seconds(20);
        /// How long to wait for a PING ACK before declaring the peer dead.
        Duration ping_timeout = std::chrono::seconds(10);
        /// Maximum number of unanswered PINGs before we give up.
        std::size_t max_inflight = 3;
        /// Size of the rolling RTT sample window.
        std::size_t rtt_window = 16;
    };

    PingTracker() noexcept : PingTracker(Config{}) {}

    // Takes an explicit Config — no defaulted argument, so no NSDMI-in-default-arg issue.
    explicit PingTracker(Config cfg) noexcept : m_cfg{cfg}, m_last_activity{Clock::now()} {
        m_rtt_window.reserve(m_cfg.rtt_window);
    }

    /// Generate a new PING payload and record the send time.
    /// Returns std::nullopt if we already have max_inflight unanswered pings.
    [[nodiscard]]
    std::optional<Payload> make_ping(TimePoint now = Clock::now()) {
        if (m_inflight.size() >= m_cfg.max_inflight)
            return std::nullopt;

        Payload p = random_payload();
        m_inflight.emplace(payload_key(p), PendingPing{now});
        m_last_ping_sent = now;
        return p;
    }


    /// Call when a PING+ACK frame is received with this payload.
    /// Returns true if the payload matched a pending ping (expected case).
    bool on_ack(const Payload &payload, TimePoint now = Clock::now()) {
        auto key = payload_key(payload);
        auto it = m_inflight.find(key);
        if (it == m_inflight.end())
            return false; // unsolicited or duplicate ACK — caller may log

        Duration rtt = now - it->second.sent_at;
        m_inflight.erase(it);
        record_rtt(rtt);
        note_activity(now);
        return true;
    }


    /// Call from the I/O loop at regular intervals.
    /// Returns an Action the session should perform, or nullopt if nothing needed.
    [[nodiscard]]
    std::optional<Action> check_keepalive(TimePoint now = Clock::now()) {
        // 1. Check for timed-out inflight pings → GOAWAY
        for (auto &[key, pending] : m_inflight) {
            if (now - pending.sent_at >= m_cfg.ping_timeout) {
                m_timed_out = true;
                return Action{ActionKind::SendGoaway};
            }
        }

        // 2. If connection is idle and no ping in flight → send keepalive PING
        bool idle = (now - m_last_activity) >= m_cfg.keepalive_interval;
        bool no_ping = m_inflight.empty();
        if (idle && no_ping) {
            if (auto p = make_ping(now))
                return Action{ActionKind::SendPing, *p};
        }

        return std::nullopt;
    }

    /// Call whenever any non-PING data is sent or received to reset the idle clock.
    void note_activity(TimePoint now = Clock::now()) noexcept { m_last_activity = now; }

    [[nodiscard]] const RttStats &rtt_stats() const noexcept { return m_stats; }
    [[nodiscard]] bool timed_out() const noexcept { return m_timed_out; }
    [[nodiscard]] std::size_t inflight() const noexcept { return m_inflight.size(); }

    /// Latest RTT samples (bounded ring, oldest first).
    [[nodiscard]] std::span<const Duration> rtt_window() const noexcept { return m_rtt_window; }

  private:
    struct PendingPing {
        TimePoint sent_at;
    };

    static std::uint64_t payload_key(const Payload &p) noexcept {
        std::uint64_t k{};
        std::memcpy(&k, p.data(), 8);
        return k;
    }

    static Payload random_payload() noexcept {
        // std::mt19937_64 seeded once per tracker — cheap, non-crypto.
        static thread_local std::mt19937_64 rng{
            static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())};
        std::uniform_int_distribution<std::uint64_t> dist;
        std::uint64_t v = dist(rng);
        Payload p{};
        std::memcpy(p.data(), &v, 8);
        return p;
    }

    void record_rtt(Duration rtt) {
        using namespace std::chrono;

        // Update rolling window (bounded by rtt_window capacity).
        if (m_rtt_window.size() == m_cfg.rtt_window)
            m_rtt_window.erase(m_rtt_window.begin());
        m_rtt_window.push_back(rtt);

        // EWMA — same constants as RFC 6298 (TCP).
        constexpr double alpha = 0.125;
        constexpr double beta = 0.25;

        if (m_stats.samples == 0) {
            m_stats.srtt = rtt;
            m_stats.rttvar = rtt / 2;
        } else {
            auto srtt_d = duration_cast<duration<double>>(m_stats.srtt);
            auto rtt_d = duration_cast<duration<double>>(rtt);
            auto var_d = duration_cast<duration<double>>(m_stats.rttvar);

            double diff = std::abs((rtt_d - srtt_d).count());
            var_d = duration<double>{(1 - beta) * var_d.count() + beta * diff};
            srtt_d = duration<double>{(1 - alpha) * srtt_d.count() + alpha * rtt_d.count()};

            m_stats.srtt = duration_cast<Duration>(srtt_d);
            m_stats.rttvar = duration_cast<Duration>(var_d);
        }

        m_stats.min_rtt = std::min(m_stats.min_rtt, rtt);
        m_stats.max_rtt = std::max(m_stats.max_rtt, rtt);
        ++m_stats.samples;
    }


    Config m_cfg;
    std::map<std::uint64_t, PendingPing> m_inflight;
    TimePoint m_last_activity;
    TimePoint m_last_ping_sent{};
    RttStats m_stats;
    std::vector<Duration> m_rtt_window;
    bool m_timed_out{false};
};

} // namespace transport::shared_layer::ping

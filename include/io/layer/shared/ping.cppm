export module io_layer_shared:ping;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_layer::ping {

class PingTracker {
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using Payload = std::vector<std::byte>;

    struct RttStats {
        Duration m_srtt{};   ///< smoothed RTT (EWMA, α=0.125)
        Duration m_rttvar{}; ///< RTT variance (EWMA, β=0.25)
        Duration m_min_rtt{Duration::max()};
        Duration m_max_rtt{};
        std::size_t m_samples{0};
    };

    enum class ActionKind : std::uint8_t { SEND_PING, SEND_GOAWAY };

    struct Action {
        ActionKind m_kind;
        Payload m_payload; ///< valid when kind == SEND_PING
    };

    struct Config {
        /// How long to wait before sending a keepalive PING when idle.
        Duration m_keepalive_interval = std::chrono::seconds(20);
        /// How long to wait for a PING ACK before declaring the peer dead.
        Duration m_ping_timeout = std::chrono::seconds(10);
        /// Maximum number of unanswered PINGs before we give up.
        std::size_t m_max_inflight = 3;
        /// Size of the rolling RTT sample window.
        std::size_t m_rtt_window = 16;
    };

    /**
     * @brief Default ctor, bet — just delegates to the Config-taking ctor with a stock Config{},
     * so you get sane defaults without having to spell them out yourself.
     */
    PingTracker() noexcept : PingTracker(Config{}) {}

    // Takes an explicit Config — no defaulted argument, so no NSDMI-in-default-arg issue.
    /**
     * @brief Builds a tracker around a caller-supplied Config, seeding the activity clock to now
     * and pre-reserving the RTT window so the first few samples don't trigger reallocs.
     * @param cfg the keepalive/timeout/window tuning knobs for this tracker.
     */
    explicit PingTracker(Config cfg) noexcept : m_cfg{cfg}, m_last_activity{Clock::now()} {
        m_rtt_window.reserve(m_cfg.m_rtt_window);
    }

    /// Generate a new PING payload and record the send time.
    /// Returns std::nullopt if we already have max_inflight unanswered pings.
    /**
     * @brief Mints a fresh random PING payload and files it as inflight, unless we're already
     * capped out — max_inflight's the ceiling, no exceptions, that's the L you eat if you're
     * already maxed.
     * @param now the current time, used to timestamp the pending ping; defaults to
     * `Clock::now()`.
     * @return the new payload to send, or `std::nullopt` if `max_inflight` unanswered pings are
     * already outstanding.
     */
    [[nodiscard]]
    std::optional<Payload> make_ping(TimePoint now = Clock::now()) {
        // Guard clause — already maxed on unanswered pings, no new one goes out.
        if (m_inflight.size() >= m_cfg.m_max_inflight) {
            return std::nullopt;
        }

        // Mint the payload and file it as pending so on_ack() can match it later.
        Payload ping_payload = random_payload();
        m_inflight.emplace(payload_key(ping_payload), PendingPing{now});
        m_last_ping_sent = now;
        return ping_payload;
    }


    /// Call when a PING+ACK frame is received with this payload.
    /// Returns true if the payload matched a pending ping (expected case).
    /**
     * @brief Matches an incoming PING ACK against the inflight table, and if it hits, records
     * the round-trip time and marks the connection alive again.
     * @param payload the ACK's payload, matched against what `make_ping()` sent out.
     * @param now the current time, used to compute the RTT sample; defaults to `Clock::now()`.
     * @return true if `payload` matched a pending ping (the expected case); false for an
     * unsolicited or duplicate ACK, which the caller may want to log.
     */
    bool on_ack(const Payload &payload, TimePoint now = Clock::now()) {
        // Look up the pending ping this ACK is supposedly answering.
        auto key = payload_key(payload);
        auto it = m_inflight.find(key);
        if (it == m_inflight.end()) {
            return false; // unsolicited or duplicate ACK — caller may log
        }

        // Match found — clock the RTT, clear it from inflight, and bump the idle timer.
        Duration rtt = now - it->second.m_sent_at;
        m_inflight.erase(it);
        record_rtt(rtt);
        note_activity(now);
        return true;
    }


    /// Call from the I/O loop at regular intervals.
    /// Returns an Action the session should perform, or nullopt if nothing needed.
    /**
     * @brief Ticks the keepalive state machine — first checks if any inflight ping has blown
     * past its timeout (peer's cooked, time for GOAWAY), then checks if the connection's gone
     * idle long enough to warrant a fresh keepalive PING.
     * @param now the current time to check timeouts/idleness against; defaults to
     * `Clock::now()`.
     * @return `Action{SendGoaway}` if an inflight ping timed out, `Action{SendPing, payload}` if
     * idle with nothing inflight, or `std::nullopt` if there's nothing to do right now.
     */
    [[nodiscard]]
    std::optional<Action> check_keepalive(TimePoint now = Clock::now()) {
        // 1. Check for timed-out inflight pings → GOAWAY
        for (auto &[key, pending] : m_inflight) {
            if (now - pending.m_sent_at >= m_cfg.m_ping_timeout) {
                m_timed_out = true;
                return Action{.m_kind = ActionKind::SEND_GOAWAY};
            }
        }

        // 2. If connection is idle and no ping in flight → send keepalive PING
        bool idle = (now - m_last_activity) >= m_cfg.m_keepalive_interval;
        bool no_ping = m_inflight.empty();
        if (idle && no_ping) {
            if (auto ping_payload = make_ping(now)) {
                return Action{.m_kind = ActionKind::SEND_PING, .m_payload = *ping_payload};
            }
        }

        return std::nullopt;
    }

    /// Call whenever any non-PING data is sent or received to reset the idle clock.
    /**
     * @brief Bumps the idle clock — call this on any non-PING traffic so `check_keepalive()`
     * doesn't think the connection went quiet when it didn't.
     * @param now the new "last activity" timestamp; defaults to `Clock::now()`.
     */
    void note_activity(TimePoint now = Clock::now()) noexcept { m_last_activity = now; }

    /**
     * @brief Gets the rolling RTT stats — smoothed RTT, variance, min/max, sample count. Tight
     * numbers here are the W, means the link's behaving.
     * @return the current RttStats snapshot.
     */
    [[nodiscard]] const RttStats &rtt_stats() const noexcept { return m_stats; }
    /**
     * @brief Checks whether this tracker's declared the peer dead off a blown ping timeout —
     * once this flips there's no walking it back, GOAWAY's already the call.
     * @return true once `check_keepalive()` has flagged a timeout, false otherwise.
     */
    [[nodiscard]] bool timed_out() const noexcept { return m_timed_out; }
    /**
     * @brief Gets how many PINGs are currently awaiting an ACK.
     * @return the count of pending pings.
     */
    [[nodiscard]] std::size_t inflight() const noexcept { return m_inflight.size(); }

    /// Latest RTT samples (bounded ring, oldest first).
    /**
     * @brief Gets the rolling RTT sample window.
     * @return the latest RTT samples in a bounded ring, oldest sample first.
     */
    [[nodiscard]] std::span<const Duration> rtt_window() const noexcept { return m_rtt_window; }

  private:
    struct PendingPing {
        TimePoint m_sent_at;
    };

    /**
     * @brief Pulls the first 8 bytes of a ping payload out as a plain integer key — cheap way to
     * stash/match payloads in the inflight map without hashing the whole vector every time.
     * @param payload the payload to key; must be at least 8 bytes (payloads always are, see
     * random_payload()).
     * @return the first 8 bytes of `payload`, memcpy'd into a `std::uint64_t`.
     */
    static std::uint64_t payload_key(const Payload &payload) noexcept {
        std::uint64_t key_value{};
        std::memcpy(&key_value, payload.data(), 8);
        return key_value;
    }

    /**
     * @brief Rolls a fresh 8-byte random payload for a PING frame — not crypto-grade, just needs
     * to be unpredictable enough that an ACK obviously matches the ping that caused it.
     * @return an 8-byte payload seeded from a thread-local Mersenne Twister.
     */
    static Payload random_payload() noexcept {
        // std::mt19937_64 seeded once per tracker — cheap, non-crypto.
        static thread_local std::mt19937_64 rng{
            static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())};
        // Draw a random 64-bit value and copy its bytes straight into the payload.
        std::uniform_int_distribution<std::uint64_t> dist;
        std::uint64_t random_value = dist(rng);
        Payload payload_bytes{};
        std::memcpy(payload_bytes.data(), &random_value, 8);
        return payload_bytes;
    }

    /**
     * @brief Folds a fresh RTT sample into the rolling window plus the smoothed RTT/variance
     * EWMA — lowkey the same alpha/beta constants TCP uses per RFC 6298, so the smoothing
     * behaves the way everyone already expects.
     * @param rtt the just-measured round-trip time to fold in.
     */
    void record_rtt(Duration rtt) {
        using namespace std::chrono;

        // Update rolling window (bounded by rtt_window capacity).
        if (m_rtt_window.size() == m_cfg.m_rtt_window) {
            m_rtt_window.erase(m_rtt_window.begin());
        }
        m_rtt_window.push_back(rtt);

        // EWMA — same constants as RFC 6298 (TCP).
        constexpr double ALPHA = 0.125;
        constexpr double BETA = 0.25;

        if (m_stats.m_samples == 0) {
            m_stats.m_srtt = rtt;
            m_stats.m_rttvar = rtt / 2;
        } else {
            auto srtt_d = duration_cast<duration<double>>(m_stats.m_srtt);
            auto rtt_d = duration_cast<duration<double>>(rtt);
            auto var_d = duration_cast<duration<double>>(m_stats.m_rttvar);

            double diff = std::abs((rtt_d - srtt_d).count());
            var_d = duration<double>{((1 - BETA) * var_d.count()) + (BETA * diff)};
            srtt_d = duration<double>{((1 - ALPHA) * srtt_d.count()) + (ALPHA * rtt_d.count())};

            m_stats.m_srtt = duration_cast<Duration>(srtt_d);
            m_stats.m_rttvar = duration_cast<Duration>(var_d);
        }

        m_stats.m_min_rtt = std::min(m_stats.m_min_rtt, rtt);
        m_stats.m_max_rtt = std::max(m_stats.m_max_rtt, rtt);
        ++m_stats.m_samples;
    }


    Config m_cfg;
    std::map<std::uint64_t, PendingPing> m_inflight;
    TimePoint m_last_activity;
    TimePoint m_last_ping_sent;
    RttStats m_stats;
    std::vector<Duration> m_rtt_window;
    bool m_timed_out{false};
};

} // namespace io::shared_layer::ping

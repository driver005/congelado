module io.layer.shared.ping;
@nogc nothrow:

// PORT-NOTE: namespace io::shared_layer::ping → module io.layer.shared.ping.
// C++ used std::chrono::steady_clock, std::map, std::vector (all heap/GC).
// D port uses:
//   - MonoTime / Duration from core.time for timestamps (clock-free arithmetic)
//   - Fixed-size arrays for inflight pings and RTT window (bounded, @nogc-safe)
//   - Manual linear scan replacing std::map (small N; SwissHashMap upgrade in Run 3)

import core.time : MonoTime, Duration, dur, seconds, msecs;

/// Maximum number of in-flight pings tracked simultaneously.
enum size_t MAX_INFLIGHT_PINGS = 8;
/// Maximum RTT window size.
enum size_t MAX_RTT_WINDOW = 16;
/// Ping payload is always 8 bytes (HTTP/2 spec).
enum size_t PING_PAYLOAD_SIZE = 8;

/// io::shared_layer::ping::PingTracker::RttStats
struct RttStats {
    Duration srtt;                          ///< smoothed RTT (EWMA, α=0.125)
    Duration rttvar;                        ///< RTT variance (EWMA, β=0.25)
    Duration min_rtt = dur!"seconds"(long.max); ///< initialized to max
    Duration max_rtt;
    size_t   samples;
}

/// io::shared_layer::ping::PingTracker::ActionKind
enum ActionKind : ubyte { SendPing, SendGoaway }

/// io::shared_layer::ping::PingTracker::Action
struct Action {
    ActionKind kind;
    ubyte[PING_PAYLOAD_SIZE] payload; ///< valid when kind == SendPing
}

/// io::shared_layer::ping::PingTracker::Config
struct Config {
    /// How long to wait before sending a keepalive PING when idle.
    Duration keepalive_interval = dur!"seconds"(20);
    /// How long to wait for a PING ACK before declaring the peer dead.
    Duration ping_timeout = dur!"seconds"(10);
    /// Maximum number of unanswered PINGs before we give up.
    size_t max_inflight = 3;
    /// Size of the rolling RTT sample window.
    size_t rtt_window = 16;
}

// PORT-NOTE: C++ PendingPing was a struct inside PingTracker.
private struct PendingPing {
    ulong key;            ///< first 8 bytes of payload as uint64
    MonoTime sent_at;
    bool valid;           ///< slot is occupied
}

/// io::shared_layer::ping::PingTracker
class PingTracker {
  public:
    this() {
        m_cfg = Config.init;
        m_last_activity = MonoTime.currTime;
        m_timed_out = false;
        m_inflight_count = 0;
        m_rtt_window_count = 0;
        m_stats = RttStats.init;
        m_stats.min_rtt = dur!"seconds"(long.max / 1_000_000_000L);
    }

    this(Config cfg) {
        m_cfg = cfg;
        m_last_activity = MonoTime.currTime;
        m_timed_out = false;
        m_inflight_count = 0;
        m_rtt_window_count = 0;
        m_stats = RttStats.init;
        m_stats.min_rtt = dur!"seconds"(long.max / 1_000_000_000L);
    }

    /// Generate a new PING payload and record the send time.
    /// Returns false (and leaves payload unchanged) if max_inflight reached.
    bool make_ping(MonoTime now, ref ubyte[PING_PAYLOAD_SIZE] payload) {
        if (m_inflight_count >= m_cfg.max_inflight)
            return false;

        random_payload(payload);
        ulong key = payload_key(payload);

        // Find free slot
        foreach (ref slot; m_inflight) {
            if (!slot.valid) {
                slot = PendingPing(key, now, true);
                ++m_inflight_count;
                m_last_ping_sent = now;
                return true;
            }
        }
        return false; // all slots full (max_inflight enforced above)
    }

    /// Call when a PING+ACK frame is received with this payload.
    /// Returns true if the payload matched a pending ping.
    bool on_ack(const(ubyte)[] payload, MonoTime now) {
        if (payload.length < PING_PAYLOAD_SIZE)
            return false;
        ulong key = payload_key(payload[0..PING_PAYLOAD_SIZE]);
        foreach (ref slot; m_inflight) {
            if (slot.valid && slot.key == key) {
                Duration rtt = now - slot.sent_at;
                slot.valid = false;
                --m_inflight_count;
                record_rtt(rtt);
                note_activity(now);
                return true;
            }
        }
        return false;
    }

    /// Call from I/O loop at regular intervals.
    /// Returns a valid Action if one is needed, or Action with kind=-1 (check has_action).
    bool check_keepalive(MonoTime now, ref Action out_action) {
        // 1. Check for timed-out in-flight pings → GOAWAY
        foreach (ref slot; m_inflight) {
            if (slot.valid && (now - slot.sent_at) >= m_cfg.ping_timeout) {
                m_timed_out = true;
                out_action = Action(ActionKind.SendGoaway);
                return true;
            }
        }

        // 2. Idle + no inflight → send keepalive PING
        bool idle = (now - m_last_activity) >= m_cfg.keepalive_interval;
        bool no_ping = (m_inflight_count == 0);
        if (idle && no_ping) {
            ubyte[PING_PAYLOAD_SIZE] p;
            if (make_ping(now, p)) {
                out_action = Action(ActionKind.SendPing);
                out_action.payload = p;
                return true;
            }
        }
        return false;
    }

    void note_activity(MonoTime now) { m_last_activity = now; }

    ref const(RttStats) rtt_stats() const { return m_stats; }
    bool timed_out() const { return m_timed_out; }
    size_t inflight() const { return m_inflight_count; }

  private:
    static ulong payload_key(const(ubyte)[] p) {
        ulong k = 0;
        foreach (i; 0..PING_PAYLOAD_SIZE) {
            k = (k << 8) | p[i];
        }
        return k;
    }

    // PORT-NOTE: C++ used thread_local std::mt19937_64; D port uses a simple
    // xorshift64 seeded from MonoTime to stay @nogc.
    static ulong s_rng_state = 0x9E3779B97F4A7C15UL; // arbitrary non-zero seed

    static void random_payload(ref ubyte[PING_PAYLOAD_SIZE] out_) {
        // xorshift64
        ulong x = s_rng_state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        s_rng_state = x;
        foreach (i; 0..PING_PAYLOAD_SIZE) {
            out_[i] = cast(ubyte)(x >> (i * 8));
        }
    }

    void record_rtt(Duration rtt) {
        // Rolling window (bounded by MAX_RTT_WINDOW).
        if (m_rtt_window_count < MAX_RTT_WINDOW) {
            m_rtt_window[m_rtt_window_count++] = rtt;
        } else {
            // Shift left — drop oldest entry.
            foreach (i; 0..MAX_RTT_WINDOW - 1) {
                m_rtt_window[i] = m_rtt_window[i + 1];
            }
            m_rtt_window[MAX_RTT_WINDOW - 1] = rtt;
        }

        // EWMA — same constants as RFC 6298 (TCP).
        // PORT-NOTE: Duration arithmetic in D uses hnsecs; convert to a common unit.
        if (m_stats.samples == 0) {
            m_stats.srtt   = rtt;
            m_stats.rttvar = rtt / 2;
        } else {
            // Use nanosecond counts for floating-point EWMA.
            long srtt_ns   = m_stats.srtt.total!"nsecs";
            long rtt_ns    = rtt.total!"nsecs";
            long var_ns    = m_stats.rttvar.total!"nsecs";

            enum double alpha = 0.125;
            enum double beta  = 0.25;

            double diff = rtt_ns > srtt_ns ? rtt_ns - srtt_ns : srtt_ns - rtt_ns;
            var_ns  = cast(long)((1.0 - beta)  * var_ns  + beta  * diff);
            srtt_ns = cast(long)((1.0 - alpha) * srtt_ns + alpha * rtt_ns);

            m_stats.srtt   = dur!"nsecs"(srtt_ns);
            m_stats.rttvar = dur!"nsecs"(var_ns);
        }

        if (rtt < m_stats.min_rtt) m_stats.min_rtt = rtt;
        if (rtt > m_stats.max_rtt) m_stats.max_rtt = rtt;
        ++m_stats.samples;
    }

    Config   m_cfg;
    PendingPing[MAX_INFLIGHT_PINGS] m_inflight;
    size_t   m_inflight_count;
    MonoTime m_last_activity;
    MonoTime m_last_ping_sent;
    RttStats m_stats;
    Duration[MAX_RTT_WINDOW] m_rtt_window;
    size_t   m_rtt_window_count;
    bool     m_timed_out;
}

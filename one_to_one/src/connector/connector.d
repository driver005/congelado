module connector.connector;

@nogc nothrow:

public import connector.local_cache;

import interfaces.cache    : ICache;
import interfaces.database : IDatabase;
import shared_.handler     : HandlerBase, WorkerFunction, QueryReadFn;
import serde.core          : is_connectable;
import serde.json          : Json;
import serde.cache         : Cache;
import serde.sql           : Sql, QueryOptions;
import util.optional       : Optional;
import util.result         : Result;
import util.alloc          : make;

// PORT-NOTE: C++ used std::queue<std::move_only_function<void()>> for m_pending,
// std::unordered_map<std::type_index, std::any> for m_local_stores, and
// std::vector<T> in several places. D port uses fixed-size arrays+count.
// Max 256 pending operations, one local store slot per concrete type (hardcoded).
// IMPROVEMENTS: wire a proper queue from utils.queue in Run 3.

// Callback type for completion notification.
alias CompletionCb = void function(bool) @nogc nothrow;

// Pending operation — stored as fn+ctx pair to avoid closures.
private struct PendingOp {
    void function(void*) @nogc nothrow fn;
    void* ctx;
}

class Connector : HandlerBase {
  public:
    this() nothrow {}
    this(ICache cache, IDatabase database) nothrow {
        m_cache    = cache;
        m_database = database;
    }

    void set_cache(ICache cache)       nothrow { m_cache    = cache;    }
    void set_database(IDatabase database) nothrow { m_database = database; }

    override const(char)[] get_name() const nothrow { return "connector"; }

    override WorkerFunction on_execute() override {
        // PORT-NOTE: C++ lambda captured [this] and processed one pending op per tick.
        // D: return a function pointer + context pair (fn, this).
        // TODO: wire pending queue draining in Run 2.
        return WorkerFunction(null, null); // stub
    }

    // ─── create_table ──────────────────────────────────────────────────────────
    // PORT-NOTE: C++ used move_only_function<void(bool)> callback; D uses fn+ctx.
    void create_table(T)(void function(bool, void*) @nogc nothrow callback,
                         void* callback_ctx) nothrow
            if (is_connectable!T) {
        if (m_database is null) {
            callback(true, callback_ctx);
            return;
        }
        // TODO: call active_database().query(Sql.build_create_sql!T(), ...) in Run 2.
        callback(false, callback_ctx);
    }

    // ─── find ─────────────────────────────────────────────────────────────────
    // PORT-NOTE: C++ used std::optional<T> callback; D uses fn(T*, void*) with null=not found.
    void find(T)(const(char)[] key,
                 void function(T*, void*) @nogc nothrow callback,
                 void* callback_ctx) nothrow
            if (is_connectable!T) {
        if (m_database is null) {
            // local store lookup — TODO wire in Run 2
            callback(null, callback_ctx);
            return;
        }
        // TODO: cache check → DB query in Run 2.
        callback(null, callback_ctx);
    }

    // ─── insert ───────────────────────────────────────────────────────────────
    void insert(T)(const ref T value,
                   void function(bool, void*) @nogc nothrow callback,
                   void* callback_ctx) nothrow
            if (is_connectable!T) {
        // TODO: write_through in Run 2
        callback(false, callback_ctx); // stub
    }

    // ─── update ───────────────────────────────────────────────────────────────
    void update(T)(const ref T value,
                   void function(bool, void*) @nogc nothrow callback,
                   void* callback_ctx) nothrow
            if (is_connectable!T) {
        callback(false, callback_ctx); // stub
    }

    // ─── upsert ───────────────────────────────────────────────────────────────
    void upsert(T)(const ref T value,
                   void function(bool, void*) @nogc nothrow callback,
                   void* callback_ctx) nothrow
            if (is_connectable!T) {
        callback(false, callback_ctx); // stub
    }

    // ─── remove ───────────────────────────────────────────────────────────────
    void remove(T)(const(char)[] key,
                   void function(bool, void*) @nogc nothrow callback,
                   void* callback_ctx) nothrow
            if (is_connectable!T) {
        active_cache().remove(Cache.cache_key_by_pk!T(key),
                              cast(QueryReadFn) null); // TODO wire properly
        callback(true, callback_ctx); // stub
    }

  private:
    ICache     active_cache()    nothrow { return m_cache !is null ? m_cache : m_local_cache; }
    IDatabase  active_database() nothrow { return m_database; }

    // PORT-NOTE: C++ used std::queue<move_only_function> for pending ops.
    // D uses a fixed circular buffer in Run 2.
    PendingOp[256] m_pending_buf;
    size_t         m_pending_head;
    size_t         m_pending_tail;

    ICache     m_cache    = null;
    IDatabase  m_database = null;
    LocalCache m_local_cache;

    // PORT-NOTE: C++ used unordered_map<type_index, any> for per-type local stores.
    // D port defers per-type stores to Run 2 (no std::any / type_index equivalent).
}

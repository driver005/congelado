module serde.sql;

@nogc nothrow:

import serde.core : is_connectable, Serializable;
import serde.json : Json;
import util.optional : Optional;

// PORT-NOTE: C++ built SQL strings via std::format + constexpr template iteration
// over FieldDesc tuples. D port stubs all builders (Run 2 will implement using
// snprintf into stack buffers once field iteration is available).

// ─── QueryOptions ─────────────────────────────────────────────────────────────

// PORT-NOTE: C++ used std::vector for joins, where_conditions, order_by_clauses.
// D uses fixed-size arrays+count.

struct OrderClause {
    const(char)[] column;
    bool          ascending = true;
}

class QueryOptions {
  public:
    QueryOptions add_join(const(char)[] join_) nothrow {
        assert(m_joins_count < 16);
        m_joins_buf[m_joins_count++] = join_;
        return this;
    }
    QueryOptions add_where(const(char)[] condition) nothrow {
        assert(m_where_count < 16);
        m_where_buf[m_where_count++] = condition;
        return this;
    }
    QueryOptions add_order_by(const(char)[] column, bool ascending = true) nothrow {
        assert(m_order_count < 16);
        m_order_buf[m_order_count++] = OrderClause(column, ascending);
        return this;
    }
    QueryOptions set_limit(size_t limit) nothrow {
        m_limit = Optional!size_t(limit);
        return this;
    }

    const(const(char)[])[16] get_joins()             const nothrow { return m_joins_buf;  }
    size_t                   get_joins_count()        const nothrow { return m_joins_count; }
    const(const(char)[])[16] get_where_conditions()  const nothrow { return m_where_buf;  }
    size_t                   get_where_count()        const nothrow { return m_where_count; }
    const(OrderClause)[16]   get_order_by_clauses()  const nothrow { return m_order_buf;  }
    size_t                   get_order_count()        const nothrow { return m_order_count; }
    const(Optional!size_t)   get_limit()              const nothrow { return m_limit;       }

  private:
    // PORT-NOTE: C++ used std::vector; D uses fixed-size buffers.
    const(char)[][16]  m_joins_buf;
    size_t             m_joins_count;
    const(char)[][16]  m_where_buf;
    size_t             m_where_count;
    OrderClause[16]    m_order_buf;
    size_t             m_order_count;
    Optional!size_t    m_limit;
}

// ─── Sql ──────────────────────────────────────────────────────────────────────
// PORT-NOTE: All methods are stubs — C++ used std::format + FieldDesc tuple iteration.
// Run 2 will implement using snprintf + Serializable!T.fields() iteration.

class Sql {
  public:
    // TODO: implement in Run 2 with @nogc string building.
    static const(char)[] build_create_sql(T)() nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_select_sql(T)(const(char)[] key) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_select_many_sql(T)(const(char)[][] keys) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_select_all_sql(T)() nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_insert_sql(T)(const ref T value) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_insert_many_sql(T)(const(T)[] values) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_update_sql(T)(const ref T value) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_upsert_sql(T)(const ref T value) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_delete_sql(T)(const(char)[] key) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_delete_many_sql(T)(const(char)[][] keys) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_query_sql(T)(const ref QueryOptions options) nothrow
            if (is_connectable!T) { return ""; }

    static const(char)[] build_query_first_sql(T)(const ref QueryOptions options) nothrow
            if (is_connectable!T) { return ""; }
}

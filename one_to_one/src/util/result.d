module util.result;
@nogc nothrow:

// PORT-NOTE: value wrapper, exempt from classes-only rule.
// Mirrors std::expected<T,E>.
struct Result(T, E)
{
    private union Payload { T ok; E err; }
    private Payload m_payload;
    private bool    m_ok;

    static Result ok()(auto ref T v)
    {
        Result r;
        r.m_payload.ok = v;
        r.m_ok = true;
        return r;
    }

    static Result err()(auto ref E e)
    {
        Result r;
        r.m_payload.err = e;
        r.m_ok = false;
        return r;
    }

    bool is_ok()  const { return  m_ok; }
    bool is_err() const { return !m_ok; }

    ref inout(T) value() inout
    {
        assert(m_ok, "Result is Err");
        return m_payload.ok;
    }

    ref inout(E) error() inout
    {
        assert(!m_ok, "Result is Ok");
        return m_payload.err;
    }

    bool opCast(B : bool)() const { return m_ok; }
}

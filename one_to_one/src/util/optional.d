module util.optional;
@nogc nothrow:

// PORT-NOTE: value wrapper, exempt from classes-only rule.
// Mirrors std::optional<T>. Returns by value on hot paths.
struct Optional(T)
{
    private T    m_value;
    private bool m_has_value;

    static Optional none() { return Optional.init; }

    static Optional some()(auto ref T v)
    {
        Optional o;
        o.m_value     = v;
        o.m_has_value = true;
        return o;
    }

    bool has_value() const { return m_has_value; }

    ref inout(T) value() inout
    {
        assert(m_has_value, "optional is empty");
        return m_value;
    }

    T value_or()(auto ref T fallback) const
    {
        return m_has_value ? m_value : fallback;
    }

    bool opCast(B : bool)() const { return m_has_value; }
}

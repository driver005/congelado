module;

export module cc_abi_sonic_intern:string_hive;

import std;
import :string;

export namespace ice::sonic {

// StringHive — accumulates StringRuntime pieces into one shared StringRuntime. append() grows
// the shared value by rebuilding it (StringRuntime has no in-place mutation) — fine for the
// modest, render()-sized text this exists for; not meant for high-volume accumulation.
class StringHive
{
public:
    void append(const StringRuntime& piece)
    {
        m_shared = StringRuntime{m_shared.to_std_string() + piece.to_std_string()};
    }

    void append_newline()
    {
        append(StringRuntime{"\n"});
    }

    [[nodiscard]] const StringRuntime& get() const
    {
        return m_shared;
    }

private:
    StringRuntime m_shared;
};

} // namespace ice::sonic

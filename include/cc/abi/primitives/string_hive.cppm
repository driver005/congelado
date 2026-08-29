module;

export module cc_abi_primitives:string_hive;

import std;
import :string;

export namespace ice {

// StringHive — accumulates String pieces into one shared String. append() grows
// the shared value by rebuilding it (String has no in-place mutation) — fine for the
// modest, render()-sized text this exists for; not meant for high-volume accumulation.
class StringHive
{
public:
    void append(const String& piece)
    {
        m_shared = String{m_shared.to_std_string() + piece.to_std_string()};
    }

    void append_newline()
    {
        append(String{"\n"});
    }

    [[nodiscard]] const String& get() const
    {
        return m_shared;
    }

private:
    String m_shared;
};

} // namespace ice::sonic

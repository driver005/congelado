module;

export module cc_abi_primitives:string_hive;

import std;
import :string;

export namespace ice {

// StringHive — accumulates String pieces into one String. append() grows the shared
// buffer amortized O(1) per piece (std::string growth), so rendering N pieces is O(N),
// not O(N^2) like the previous rebuild-on-every-append implementation. Every member is
// noexcept; an allocation failure inside append() clears the buffer rather than throwing
// (no exception may escape a noexcept function — OOM is treated as "drop the render").
class StringHive
{
public:
    void append(const String& piece) noexcept
    {
        try {
            m_buf.append(piece.view());
        } catch (...) {
            m_buf.clear();
        }
    }

    void append_newline() noexcept
    {
        try {
            m_buf.push_back('\n');
        } catch (...) {
            m_buf.clear();
        }
    }

    [[nodiscard]] String get() const noexcept
    {
        return String{m_buf};
    }

private:
    std::string m_buf;
};

} // namespace ice

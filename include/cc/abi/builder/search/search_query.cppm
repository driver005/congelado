export module cc_abi_builder_search:search_query;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder::search {

// SearchQuery — owning value type the mainframe builds up before calling
// Search::search(). Pure C++, zero C-ABI/TF_* knowledge (the sonic adapter converts this into
// the raw TF_Search_Query C struct only when crossing into a cross-plugin backend).
class SearchQuery
{
public:
    SearchQuery() = default;

    SearchQuery& set_query(const ice::String& value)
    {

        m_query = value;
        return *this;
    }

    SearchQuery& set_free_text(const ice::String& value)
    {

        m_free_text = value;
        return *this;
    }

    SearchQuery& set_start(std::int64_t start)
    {

        m_start = start;
        return *this;
    }

    SearchQuery& set_size(std::int64_t size)
    {

        m_size = size;
        return *this;
    }

    SearchQuery& set_sort(const ice::String& value)
    {

        m_sort = value;
        return *this;
    }

    const ice::String& get_query() const
    {
        return m_query;
    }

    const ice::String& get_free_text() const
    {
        return m_free_text;
    }

    std::int64_t get_start() const
    {
        return m_start;
    }

    std::int64_t get_size() const
    {
        return m_size;
    }

    const ice::String& get_sort() const
    {
        return m_sort;
    }

private:
    ice::String m_query;
    ice::String m_free_text;
    std::int64_t m_start{0};
    std::int64_t m_size{10};
    ice::String m_sort;
};

} // namespace ice::builder

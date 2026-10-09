module;

export module aten_xpu_extern_kernel:onednn_primitive_cache;

import std;

export namespace aten_xpu {

template<typename Key, typename Value>
class SyclPrimitiveCache
{
public:
    explicit SyclPrimitiveCache(std::size_t capacity = 1'024) noexcept :
        m_capacity{capacity}
    {
    }

    void setCapacity(std::size_t capacity)
    {
        m_capacity = capacity;
        trim();
    }

    std::size_t getCapacity() const noexcept
    {
        return m_capacity;
    }

    std::size_t size() const noexcept
    {
        return m_index.size();
    }

    template<typename Factory>
    Value& find_or_create(const Key& key, Factory&& factory)
    {
        const std::scoped_lock lock{m_mutex};
        if (const auto found = m_index.find(key); found != m_index.end()) {
            m_entries.splice(m_entries.begin(), m_entries, found->second);
            return found->second->second;
        }

        m_entries.emplace_front(key, factory());
        m_index.emplace(key, m_entries.begin());
        trim();
        return m_entries.front().second;
    }

    void clear()
    {
        const std::scoped_lock lock{m_mutex};
        m_entries.clear();
        m_index.clear();
    }

private:
    void trim()
    {
        while (m_index.size() > m_capacity && !m_entries.empty()) {
            m_index.erase(m_entries.back().first);
            m_entries.pop_back();
        }
    }

    std::size_t m_capacity;
    std::mutex m_mutex;
    std::list<std::pair<Key, Value>> m_entries;
    std::map<Key, typename std::list<std::pair<Key, Value>>::iterator> m_index;
};

} // namespace aten_xpu

module connector.local_cache;

@nogc nothrow:

import interfaces.cache : ICache;
import shared_.handler  : QueryReadFn;

// PORT-NOTE: C++ used std::unordered_map<std::string,std::string> for m_store.
// D port uses a fixed-size KV array (256 entries) to avoid GC.
// IMPROVEMENTS: wire SwissHashMap!(const(char)[], const(char)[]) in Run 3.

private struct StoreEntry {
    const(char)[] key;
    const(char)[] value;
    bool          occupied;
}

class LocalCache : ICache {
  public:
    override const(char)[] backend_name() const nothrow { return "local"; }
    override bool required() const nothrow { return false; }

    override void get(const(char)[] key, QueryReadFn result) nothrow {
        // PORT-NOTE: C++ used unordered_map::find; D linear scan.
        foreach (ref entry; m_store[0 .. m_store_count]) {
            if (entry.occupied && entry.key == key) {
                result(entry.value);
                return;
            }
        }
        result("");
    }

    override void set(const(char)[] key, const(char)[] value,
                      QueryReadFn result) nothrow {
        // Update existing.
        foreach (ref entry; m_store[0 .. m_store_count]) {
            if (entry.occupied && entry.key == key) {
                entry.value = value;
                result("");
                return;
            }
        }
        // Insert new.
        assert(m_store_count < m_store.length);
        m_store[m_store_count++] = StoreEntry(key, value, true);
        result("");
    }

    override void remove(const(char)[] key, QueryReadFn result) nothrow {
        foreach (ref entry; m_store[0 .. m_store_count]) {
            if (entry.occupied && entry.key == key) {
                entry.occupied = false;
                break;
            }
        }
        result("");
    }

  private:
    // PORT-NOTE: C++ std::unordered_map replaced by fixed StoreEntry[256] buffer.
    StoreEntry[256] m_store;
    size_t          m_store_count;
}

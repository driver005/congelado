module;

#include "c/intern/tf_tstring.h"

export module cc_abi_runtime_intern:string;

import std;

export namespace ice {

// StringRuntime — non-owning, read-only view over a `const TF_TString*` handed to the
// mainframe by a plugin callback. No allocation, no copy, no destructor cleanup — the
// pointee's lifetime is owned by whoever passed it in (the plugin/callback caller).
class StringRuntime {
  public:
    StringRuntime() : m_str{nullptr} {}
    explicit StringRuntime(const TF_TString *str) : m_str{str} {}

    bool empty() const { return size() == 0; }
    const char *c_str() const { return m_str ? TF_StringGetDataPointer(m_str) : nullptr; }
    size_t size() const { return m_str ? TF_StringGetSize(m_str) : 0; }

    std::string to_std_string() const {
        if (!m_str) {
            return std::string();
        }
        auto *data = TF_StringGetDataPointer(m_str);
        auto sz = TF_StringGetSize(m_str);
        return data ? std::string(data, sz) : std::string();
    }

    // Underlying handle — pass directly to the C ABI
    const TF_TString *get_handle() const { return m_str; }

  private:
    const TF_TString *m_str;
};

} // namespace ice

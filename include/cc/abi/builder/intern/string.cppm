module;

#include "c/intern/tf_tstring.h"

export module cc_abi_builder_intern:string;

import std;

export namespace ice {

// StringBuilder — wraps TF_TString (C ABI string).
// No conversions needed — pass directly to C ABI functions.
//
// Usage:
//   ice::StringBuilder s("hello");
//   ice::StringBuilder s2 = s;
//   const char* cstr = s.c_str();
//
class StringBuilder {
  public:
    StringBuilder() { TF_StringInit(&m_str); }

    StringBuilder(const char *data, size_t size) {

        TF_StringInit(&m_str);
        if (data && size > 0) {
            TF_StringCopy(&m_str, data, size);
        }

    }

    explicit StringBuilder(const std::string &s) : StringBuilder(s.data(), s.size()) {}

    StringBuilder(const char *s) : StringBuilder(s, s ? std::strlen(s) : 0) {}

    explicit StringBuilder(const TF_TString *ts) {

        TF_StringInit(&m_str);
        auto *data = TF_StringGetDataPointer(ts);
        auto size = TF_StringGetSize(ts);
        if (data && size > 0) {
            TF_StringCopy(&m_str, data, size);
        }

    }

    ~StringBuilder() { TF_StringDealloc(&m_str); }

    StringBuilder(const StringBuilder &other) {

        TF_StringInit(&m_str);
        auto *view = TF_StringGetDataPointer(&other.m_str);
        auto size = TF_StringGetSize(&other.m_str);
        if (view && size > 0) {
            TF_StringCopy(&m_str, view, size);
        }

    }

    StringBuilder &operator=(const StringBuilder &other) {

        if (this != &other) {
            TF_StringDealloc(&m_str);
            TF_StringInit(&m_str);
            auto *view = TF_StringGetDataPointer(&other.m_str);
            auto size = TF_StringGetSize(&other.m_str);
            if (view && size > 0) {
                TF_StringCopy(&m_str, view, size);
            }
        }
        return *this;

    }

    StringBuilder(StringBuilder &&other) noexcept {

        m_str = other.m_str;
        TF_StringInit(&other.m_str);

    }

    StringBuilder &operator=(StringBuilder &&other) noexcept {

        if (this != &other) {
            TF_StringDealloc(&m_str);
            m_str = other.m_str;
            TF_StringInit(&other.m_str);
        }
        return *this;

    }

    const char *c_str() const { return TF_StringGetDataPointer(&m_str); }
    operator const char *() const { return c_str(); }
    size_t size() const { return TF_StringGetSize(&m_str); }
    bool empty() const { return size() == 0; }

    std::string to_std_string() const {

        auto *data = TF_StringGetDataPointer(&m_str);
        auto sz = TF_StringGetSize(&m_str);
        return data ? std::string(data, sz) : std::string();

    }

    // Underlying handle — pass directly to the C ABI
    TF_TString *get_handle() { return &m_str; }
    const TF_TString *get_handle() const { return &m_str; }

  private:
    TF_TString m_str;
};

} // namespace ice

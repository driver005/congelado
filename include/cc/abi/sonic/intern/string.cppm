module;

#include "c/intern/tf_tstring.h"

export module cc_abi_sonic_intern:string;

import std;

export namespace ice::sonic {

// StringRuntime — owns its own TF_TString. Constructing from an external `const TF_TString*`
// (a plugin callback's C struct field) or a std::string (e.g. text a render()-style method just
// built) both deep-copy into m_owned — there is no separate view/pointer state to keep track of.
class StringRuntime
{
public:
    StringRuntime()
    {
        TF_StringInit(&m_owned);
    }

    explicit StringRuntime(const TF_TString* str)
    {

        TF_StringInit(&m_owned);
        if (str) {
            auto* data = TF_StringGetDataPointer(str);
            auto size = TF_StringGetSize(str);
            if (data && size > 0) {
                TF_StringCopy(&m_owned, data, size);
            }
        }
    }

    explicit StringRuntime(const std::string& text)
    {

        TF_StringInit(&m_owned);
        if (!text.empty()) {
            TF_StringCopy(&m_owned, text.data(), text.size());
        }
    }

    ~StringRuntime()
    {
        TF_StringDealloc(&m_owned);
    }

    StringRuntime(const StringRuntime& other)
    {

        TF_StringInit(&m_owned);
        auto* data = TF_StringGetDataPointer(&other.m_owned);
        auto size = TF_StringGetSize(&other.m_owned);
        if (data && size > 0) {
            TF_StringCopy(&m_owned, data, size);
        }
    }

    StringRuntime& operator=(const StringRuntime& other)
    {

        if (this != &other) {
            TF_StringDealloc(&m_owned);
            TF_StringInit(&m_owned);
            auto* data = TF_StringGetDataPointer(&other.m_owned);
            auto size = TF_StringGetSize(&other.m_owned);
            if (data && size > 0) {
                TF_StringCopy(&m_owned, data, size);
            }
        }
        return *this;
    }

    StringRuntime(StringRuntime&& other) noexcept
    {

        m_owned = other.m_owned;
        TF_StringInit(&other.m_owned);
    }

    StringRuntime& operator=(StringRuntime&& other) noexcept
    {

        if (this != &other) {
            TF_StringDealloc(&m_owned);
            m_owned = other.m_owned;
            TF_StringInit(&other.m_owned);
        }
        return *this;
    }

    bool empty() const
    {
        return size() == 0;
    }

    const char* c_str() const
    {
        return TF_StringGetDataPointer(&m_owned);
    }

    size_t size() const
    {
        return TF_StringGetSize(&m_owned);
    }

    std::string to_std_string() const
    {

        auto* data = TF_StringGetDataPointer(&m_owned);
        auto sz = TF_StringGetSize(&m_owned);
        return data ? std::string(data, sz) : std::string();
    }

    // Underlying handle — pass directly to the C ABI
    const TF_TString* get_handle() const
    {
        return &m_owned;
    }

private:
    TF_TString m_owned;
};

} // namespace ice::sonic

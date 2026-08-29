module;

#include "c/intern/tf_tstring.h"

export module cc_abi_primitives:string;

import std;

export namespace ice {

// String — owns its own TF_TString. Constructing from an external `const TF_TString*`
// (a plugin callback's C struct field) or a std::string (e.g. text a render()-style method just
// built) both deep-copy into m_owned — there is no separate view/pointer state to keep track of.
class String
{
public:
    String()
    {
        string_init(&m_owned);
    }

    explicit String(const TF_TString* str)
    {
        string_init(&m_owned);
        if (str) {
            const char* data = string_get_data_pointer(str);
            size_t size = string_get_size(str);
            if (data && size > 0) {
                string_copy(&m_owned, data, size);
            }
        }
    }

    explicit String(std::string_view text)
    {
        string_init(&m_owned);
        if (!text.empty()) {
            string_copy(&m_owned, text.data(), text.size());
        }
    }

    ~String()
    {
        string_dealloc(&m_owned);
    }

    String(const String& other)
    {
        string_init(&m_owned);
        const char* data = string_get_data_pointer(&other.m_owned);
        size_t size = string_get_size(&other.m_owned);
        if (data && size > 0) {
            string_copy(&m_owned, data, size);
        }
    }

    String& operator=(const String& other)
    {
        if (this != &other) {
            string_dealloc(&m_owned);
            string_init(&m_owned);
            const char* data = string_get_data_pointer(&other.m_owned);
            size_t size = string_get_size(&other.m_owned);
            if (data && size > 0) {
                string_copy(&m_owned, data, size);
            }
        }
        return *this;
    }

    String(String&& other) noexcept
    {
        m_owned = other.m_owned;
        string_init(&other.m_owned);
    }

    String& operator=(String&& other) noexcept
    {
        if (this != &other) {
            string_dealloc(&m_owned);
            m_owned = other.m_owned;
            string_init(&other.m_owned);
        }
        return *this;
    }

    void to_c(TF_TString* out) const
    {
        if (out) {
            const char* data = string_get_data_pointer(&m_owned);
            size_t sz = string_get_size(&m_owned);
            string_assign_view(out, data, sz);
        }
    }

    bool empty() const
    {
        return size() == 0;
    }

    const char* c_str() const
    {
        return string_get_data_pointer(&m_owned);
    }

    size_t size() const
    {
        return string_get_size(&m_owned);
    }

    std::string to_std_string() const
    {
        const char* data = string_get_data_pointer(&m_owned);
        size_t sz = string_get_size(&m_owned);
        return data ? std::string(data, sz) : std::string();
    }

    // Underlying handle — pass directly to the C ABI
    TF_TString* get_handle()
    {
        return &m_owned;
    }

    const TF_TString* get_handle() const
    {
        return &m_owned;
    }

    // Zero-overhead alias: String is a standard-layout struct with TF_TString as its only
    // member, so a TF_TString* and a String* point to the same bytes ([basic.lval], [class.mem]).
    // This reinterpret_cast is intentional and correct; it lets callers treat a C-ABI string
    // pointer as a const String& without a copy.
    static const String& create(const TF_TString* str)
    {
        return *reinterpret_cast<const String*>(str);
    }

private:
    TF_TString m_owned;
};

} // namespace ice

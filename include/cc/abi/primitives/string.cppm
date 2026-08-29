module;

#include "c/intern/tf_tstring.h"

export module cc_abi_primitives:string;

import std;

export namespace ice {

// String — owns its own TF_TString. Constructing from an external `const TF_TString*`
// (a plugin callback's C struct field) or a std::string_view (e.g. text a render()-style
// method just built) both deep-copy into m_owned — there is no separate view/pointer state
// to keep track of.
//
// Exception contract: every member is noexcept. The backing storage is malloc-based
// (tf_tstring.cc's string_copy), so no C++ exception can originate from this class; a
// failed allocation yields an empty string, never a throw.
class String
{
public:
    String() noexcept
    {
        string_init(&m_owned);
    }

    explicit String(const TF_TString* str) noexcept
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

    explicit String(std::string_view text) noexcept
    {
        string_init(&m_owned);
        if (!text.empty()) {
            string_copy(&m_owned, text.data(), text.size());
        }
    }

    ~String() noexcept
    {
        string_dealloc(&m_owned);
    }

    String(const String& other) noexcept
    {
        string_init(&m_owned);
        const char* data = string_get_data_pointer(&other.m_owned);
        size_t size = string_get_size(&other.m_owned);
        if (data && size > 0) {
            string_copy(&m_owned, data, size);
        }
    }

    String& operator=(const String& other) noexcept
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

    // Deep-copy this String into `out` (replacing whatever `out` held). This is the
    // default for crossing the C ABI: the recipient owns the bytes and may outlive the
    // source String — e.g. a plugin vtable callback writing into a host-provided
    // TF_String that the host reads back after the call returns.
    void to_c(TF_TString* out) const noexcept
    {
        if (out) {
            string_copy(out, string_get_data_pointer(&m_owned), string_get_size(&m_owned));
        }
    }

    // Non-owning view: `out` aliases this String's buffer and is valid ONLY while *this
    // lives. Use exclusively for in-call-only out-params (consumed before the callback
    // returns); never for results the peer reads after the call.
    void to_c_view(TF_TString* out) const noexcept
    {
        if (out) {
            string_assign_view(out, string_get_data_pointer(&m_owned), string_get_size(&m_owned));
        }
    }

    bool empty() const noexcept
    {
        return size() == 0;
    }

    const char* c_str() const noexcept
    {
        return string_get_data_pointer(&m_owned);
    }

    size_t size() const noexcept
    {
        return string_get_size(&m_owned);
    }

    // Non-allocating read-only view of the contents.
    std::string_view view() const noexcept
    {
        return {c_str(), size()};
    }

    // Convenience for the C++ domain. Allocates (std::string) — do not call from a
    // noexcept ABI callback; use view() there.
    std::string to_std_string() const
    {
        const char* data = string_get_data_pointer(&m_owned);
        size_t sz = string_get_size(&m_owned);
        return data ? std::string(data, sz) : std::string();
    }

    // Underlying handle — pass directly to the C ABI.
    TF_TString* get_handle() noexcept
    {
        return &m_owned;
    }

    const TF_TString* get_handle() const noexcept
    {
        return &m_owned;
    }

    // Zero-overhead alias: String is a standard-layout struct with TF_TString as its only
    // member, so a TF_TString* and a String* point to the same bytes
    // ([basic.compound]/4, [class.mem]). This reinterpret_cast is intentional and correct;
    // it lets callers treat a C-ABI string pointer as a const String& without a copy.
    // The static_asserts below pin the layout so a future member addition fails loudly.
    static const String& create(const TF_TString* str) noexcept
    {
        return *reinterpret_cast<const String*>(str);
    }

private:
    TF_TString m_owned;
};

static_assert(std::is_standard_layout_v<String>, "String must stay standard-layout for String::create");
static_assert(sizeof(String) == sizeof(TF_TString), "String must not grow relative to TF_TString");
static_assert(alignof(String) == alignof(TF_TString), "String must keep TF_TString's alignment");

} // namespace ice

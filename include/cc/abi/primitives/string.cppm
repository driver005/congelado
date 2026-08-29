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
private:
    struct RuntimeState {
        TF_TStringOps* ops;
        void* host_context;
    };

    RuntimeState state() const {
        RuntimeState s;
        TF_InitTString(&s.ops, &s.host_context);
        return s;
    }

public:
    String()
    {
        state().ops->TF_StringInit(state().host_context, &m_owned);
    }

    explicit String(const TF_TString* str)
    {

        state().ops->TF_StringInit(state().host_context, &m_owned);
        if (str) {
            auto* data = state().ops->TF_StringGetDataPointer(state().host_context, str);
            auto size = state().ops->TF_StringGetSize(state().host_context, str);
            if (data && size > 0) {
                state().ops->TF_StringCopy(state().host_context, &m_owned, data, size);
            }
        }
    }

    explicit String(std::string_view text)
    {
        state().ops->TF_StringInit(state().host_context, &m_owned);
        if (!text.empty()) {
            state().ops->TF_StringCopy(state().host_context, &m_owned, text.data(), text.size());
        }
    }

    explicit String(const std::string& text)
    {

        state().ops->TF_StringInit(state().host_context, &m_owned);
        if (!text.empty()) {
            state().ops->TF_StringCopy(state().host_context, &m_owned, text.data(), text.size());
        }
    }

    ~String()
    {
        state().ops->TF_StringDealloc(state().host_context, &m_owned);
    }

    String(const String& other)
    {

        state().ops->TF_StringInit(state().host_context, &m_owned);
        auto* data = state().ops->TF_StringGetDataPointer(state().host_context, &other.m_owned);
        auto size = state().ops->TF_StringGetSize(state().host_context, &other.m_owned);
        if (data && size > 0) {
            state().ops->TF_StringCopy(state().host_context, &m_owned, data, size);
        }
    }

    String& operator=(const String& other)
    {

        if (this != &other) {
            state().ops->TF_StringDealloc(state().host_context, &m_owned);
            state().ops->TF_StringInit(state().host_context, &m_owned);
            auto* data = state().ops->TF_StringGetDataPointer(state().host_context, &other.m_owned);
            auto size = state().ops->TF_StringGetSize(state().host_context, &other.m_owned);
            if (data && size > 0) {
                state().ops->TF_StringCopy(state().host_context, &m_owned, data, size);
            }
        }
        return *this;
    }

    String(String&& other) noexcept
    {

        m_owned = other.m_owned;
        state().ops->TF_StringInit(state().host_context, &other.m_owned);
    }

    String& operator=(String&& other) noexcept
    {

        if (this != &other) {
            state().ops->TF_StringDealloc(state().host_context, &m_owned);
            m_owned = other.m_owned;
            state().ops->TF_StringInit(state().host_context, &other.m_owned);
        }
        return *this;
    }


    void to_c(TF_TString* out) const
    {
        if (out) {
            auto* data = state().ops->TF_StringGetDataPointer(state().host_context, &m_owned);
            auto sz = state().ops->TF_StringGetSize(state().host_context, &m_owned);
            state().ops->TF_StringAssignView(state().host_context, out, data, sz);
        }
    }

    bool empty() const
    {
        return size() == 0;
    }

    const char* c_str() const
    {
        return state().ops->TF_StringGetDataPointer(state().host_context, &m_owned);
    }

    size_t size() const
    {
        return state().ops->TF_StringGetSize(state().host_context, &m_owned);
    }

    std::string to_std_string() const
    {

        auto* data = state().ops->TF_StringGetDataPointer(state().host_context, &m_owned);
        auto sz = state().ops->TF_StringGetSize(state().host_context, &m_owned);
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

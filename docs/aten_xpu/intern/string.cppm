module;

#include "include/c/intern/tstring.h"

export module aten_xpu_intern:string;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A host string that owns its bytes or views caller memory (assign_view); no small-string layout.
class String : public ice::builder::String
{
public:
    String() noexcept = default;

    ~String() override = default;
    String(const String&) = delete;
    String& operator=(const String&) = delete;
    String(String&&) = delete;
    String& operator=(String&&) = delete;

    // Reads a string handed over the C ABI; the view is valid until the string changes.
    static std::string_view view_of(const ice::sonic::String& text) noexcept
    {
        const char* data = nullptr;
        std::size_t size = 0;
        text.get_data_pointer(&data);
        text.get_size(size);

        return data == nullptr ? std::string_view{} : std::string_view{data, size};
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void init() noexcept override
    {
        m_text.clear();
        m_view = {};
        m_is_view = false;
    }

    void copy(std::string_view src) noexcept override
    {
        m_text.assign(src);
        m_is_view = false;
    }

    void assign_view(std::string_view src) noexcept override
    {
        m_view = src;
        m_is_view = true;
    }

    void get_data_pointer(const char** out_data) noexcept override
    {
        *out_data = m_is_view ? m_view.data() : m_text.c_str();
    }

    void get_type(ice::sonic::TStringType& out_type) noexcept override
    {
        if (m_is_view) {
            out_type = ice::sonic::TStringType::k_view;
        } else if (m_text.size() < k_small_limit) {
            out_type = ice::sonic::TStringType::k_small;
        } else {
            out_type = ice::sonic::TStringType::k_large;
        }
    }

    void get_size(std::size_t& out_size) noexcept override
    {
        out_size = m_is_view ? m_view.size() : m_text.size();
    }

    void get_capacity(std::size_t& out_capacity) noexcept override
    {
        out_capacity = m_is_view ? 0 : m_text.capacity();
    }

    void dealloc() noexcept override
    {
        m_text = std::string{};
        m_view = {};
        m_is_view = false;
    }

private:
    static constexpr std::size_t k_small_limit = 23;

    std::string m_text;
    std::string_view m_view;
    bool m_is_view = false;
};

} // namespace aten_xpu

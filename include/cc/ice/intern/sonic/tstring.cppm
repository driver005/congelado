// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_ice_intern_sonic:tstring;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class String : public ice::sonic::Runtime<String, TF_StringOps>
{
public:
    explicit String(TF_StringOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void init() noexcept
    {
        m_ops->init(get_handle());
    }

    void copy(const char* src, size_t size) noexcept
    {
        m_ops->copy(get_handle(), src, size);
    }

    void assign_view(const char* src, size_t size) noexcept
    {
        m_ops->assign_view(get_handle(), src, size);
    }

    void get_data_pointer(const char** out_data) noexcept
    {
        m_ops->get_data_pointer(get_handle(), out_data);
    }

    void get_type(TFTStringType* out_type) noexcept
    {
        m_ops->get_type(get_handle(), out_type);
    }

    void get_size(size_t* out_size) noexcept
    {
        m_ops->get_size(get_handle(), out_size);
    }

    void get_capacity(size_t* out_capacity) noexcept
    {
        m_ops->get_capacity(get_handle(), out_capacity);
    }

    void dealloc() noexcept
    {
        m_ops->dealloc(get_handle());
    }
};

} // namespace ice::sonic

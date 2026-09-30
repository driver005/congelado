// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/generator.h"

export module cc_ice_extern_generator_builder:generator;

import std;

export namespace ice::builder {

class TF_GeneratorOps
{
public:
    TF_GeneratorOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_GeneratorOps(const TF_GeneratorOps&) = delete;
    TF_GeneratorOps& operator=(const TF_GeneratorOps&) = delete;

    static TF_GeneratorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_GeneratorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_GeneratorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_GeneratorOps*>(handle->plugin_data);
    }

    virtual ~TF_GeneratorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_GeneratorOps{
            .struct_size = TF_GENERATOR_STRUCT_SIZE,
            .destroy =
                [](TF_Generator* generator) noexcept
            {
                TF_GeneratorOps::from_handle(generator).destroy();
            },
            .get_name =
                [](TF_Generator* generator, TF_String* out_name) noexcept
            {
                TF_GeneratorOps::from_handle(generator).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },

        };
    }

    const ::TF_GeneratorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Generator& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_GeneratorOps m_vtable;
    TF_Generator m_handle;
};

} // namespace ice::builder

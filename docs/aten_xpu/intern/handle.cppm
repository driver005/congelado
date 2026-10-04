module;

export module aten_xpu_intern:handle;

import std;

export namespace aten_xpu {

class SyclHandle
{
public:
    SyclHandle() = delete;

    template<typename Concrete, typename Wrapper>
    static Concrete& resolve(const Wrapper& wrapper) noexcept
    {

        return static_cast<Concrete&>(Concrete::from_handle(wrapper.get_handle()));

    }

    template<typename Concrete, typename Handle>
    static Concrete& resolve_raw(Handle* handle) noexcept
    {

        return static_cast<Concrete&>(Concrete::from_handle(handle));

    }

    template<typename Concrete, typename Handle>
    static void attach(Handle* handle, Concrete& object) noexcept
    {

        handle->plugin_data = object.get_handle().plugin_data;

    }
};

} // namespace aten_xpu

export module cc_abi_builder_intern:ctx;

export namespace ice::builder {

// ctx_as<T> — recovers the concrete plugin pointer from the opaque void* context
// slot used by every C vtable callback.  Every builder vtable lambda receives its
// plugin instance as `void* ctx`; this named helper centralises the cast so the
// intent is explicit and grep-able instead of scattered static_casts.
template<typename T>
T* ctx_as(void* ctx) noexcept
{
    return static_cast<T*>(ctx);
}

// const overload for read-only vtable callbacks
template<typename T>
const T* ctx_as(const void* ctx) noexcept
{
    return static_cast<const T*>(ctx);
}

} // namespace ice::builder

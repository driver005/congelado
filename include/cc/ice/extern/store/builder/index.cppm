// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/index.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/index.h"

export module cc_ice_extern_store_builder:index;

import std;

export namespace ice::builder {

class TFStoreIndexOps
{
public:
    TFStoreIndexOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFStoreIndexOps(const TFStoreIndexOps&) = delete;
    TFStoreIndexOps& operator=(const TFStoreIndexOps&) = delete;

    static TFStoreIndexOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreIndexOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreIndexOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreIndexOps*>(handle->plugin_data);
    }

    virtual ~TFStoreIndexOps() = default;
    virtual void destroy() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    create(const ice::sonic::String& name, const ice::sonic::TF_MapOps& field_config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    drop(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreIndexOps{
            .struct_size = TF_TOREINDEX_STRUCT_SIZE,
            .destroy =
                [](TFStoreIndex* index) noexcept
            {
                TFStoreIndexOps::from_handle(index).destroy();
            },
            .create =
                [](TFStoreIndex* index,
                   const TF_String* name,
                   const TF_Map* field_config,
                   TF_Status* out_status) noexcept
            {
                auto res = TFStoreIndexOps::from_handle(index).create(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TF_MapOps::wrap(field_config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFStoreIndex* index, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res = TFStoreIndexOps::from_handle(index).drop(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list =
                [](TFStoreIndex* index, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto res = TFStoreIndexOps::from_handle(index).list(
                    ice::sonic::TF_VectorOps::wrap(out_names)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFStoreIndexOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreIndex& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreIndexOps m_vtable;
    TFStoreIndex m_handle;
};

} // namespace ice::builder

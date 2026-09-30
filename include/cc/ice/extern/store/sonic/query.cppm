// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/query.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/query.h"

export module cc_ice_extern_store_sonic:query;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreQueryOps : public ice::sonic::Runtime<TFStoreQueryOps, TFStoreQueryOps>
{
public:
    explicit TFStoreQueryOps(TFStoreQueryOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    run(const ice::sonic::TF_MapOps& filters,
        const ice::sonic::String& free_text,
        const ice::sonic::String& sort,
        size_t offset,
        size_t limit,
        TFStoreQueryFn completion,
        void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->run(
            get_handle(),
            filters.get_handle(),
            free_text.get_handle(),
            sort.get_handle(),
            offset,
            limit,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic

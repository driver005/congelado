// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/item.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/item.h"

export module cc_ice_extern_grappler_sonic:item;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerItemOps : public ice::sonic::Runtime<TFGrapplerItemOps, TFGrapplerItemOps>
{
public:
    explicit TFGrapplerItemOps(TFGrapplerItemOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_nodes_to_preserve_size(int* out_num_values, size_t* out_storage_size) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_nodes_to_preserve_size(
            get_handle(),
            out_num_values,
            out_storage_size,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_nodes_to_preserve_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_nodes_to_preserve_list(
            get_handle(),
            out_values,
            out_lengths,
            num_values,
            storage,
            storage_size,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_fetch_nodes_size(int* out_num_values, size_t* out_storage_size) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_fetch_nodes_size(
            get_handle(),
            out_num_values,
            out_storage_size,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_fetch_nodes_list(
        char** out_values,
        size_t* out_lengths,
        int num_values,
        void* storage,
        size_t storage_size
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_fetch_nodes_list(
            get_handle(),
            out_values,
            out_lengths,
            num_values,
            storage,
            storage_size,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic

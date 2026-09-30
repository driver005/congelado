// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"

export module cc_ice_extern_grappler_sonic:device_graph;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerDeviceGraphOps :
    public ice::sonic::Runtime<TFGrapplerDeviceGraphOps, TFGrapplerDeviceGraphOps>
{
public:
    explicit TFGrapplerDeviceGraphOps(TFGrapplerDeviceGraphOps* ops, void* plugin_context) noexcept
        :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    [[nodiscard]] std::expected<void, ice::sonic::Status> capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->capture_begin(
            get_handle(),
            capture_stream.get_handle(),
            pool_id,
            mode,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> capture_end() noexcept
    {
        ice::sonic::Status status;
        m_ops->capture_end(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> instantiate() noexcept
    {
        ice::sonic::Status status;
        m_ops->instantiate(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> replay() noexcept
    {
        ice::sonic::Status status;
        m_ops->replay(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> reset() noexcept
    {
        ice::sonic::Status status;
        m_ops->reset(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_pool(TF_PoolId* out_pool_id) noexcept
    {
        m_ops->get_pool(get_handle(), out_pool_id);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    register_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept
    {
        ice::sonic::Status status;
        m_ops->register_random_generator(get_handle(), generator.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    unregister_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept
    {
        ice::sonic::Status status;
        m_ops->unregister_random_generator(
            get_handle(),
            generator.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void enable_debug_mode() noexcept
    {
        m_ops->enable_debug_mode(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    debug_dump(const ice::sonic::String& path) noexcept
    {
        ice::sonic::Status status;
        m_ops->debug_dump(get_handle(), path.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic

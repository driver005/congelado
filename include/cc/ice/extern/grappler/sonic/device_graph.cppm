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

    [[nodiscard]] std::expected<void, ice::Status> capture_begin(
        const ice::sonic::TF_StreamOps& capture_stream,
        const TF_PoolId* pool_id,
        TF_CaptureMode mode
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> capture_end() noexcept
    {
        ice::Status status;
        m_ops->capture_end(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> instantiate() noexcept
    {
        ice::Status status;
        m_ops->instantiate(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> replay() noexcept
    {
        ice::Status status;
        m_ops->replay(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reset() noexcept
    {
        ice::Status status;
        m_ops->reset(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_pool(TF_PoolId* out_pool_id) noexcept
    {
        ice::Status status;
        m_ops->get_pool(get_handle(), out_pool_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    register_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept
    {
        ice::Status status;
        m_ops->register_random_generator(get_handle(), generator.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    unregister_random_generator(const ice::sonic::TF_RandomGeneratorOps& generator) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> enable_debug_mode() noexcept
    {
        ice::Status status;
        m_ops->enable_debug_mode(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    debug_dump(const ice::sonic::String& path) noexcept
    {
        ice::Status status;
        m_ops->debug_dump(get_handle(), path.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic

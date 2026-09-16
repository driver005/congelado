// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/map/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/map/map.h"

export module cc_abi_sonic_map;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Map : public ice::sonic::Runtime<Map, TF_Map>
{
public:
    explicit Map(TF_Map* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "map";

    [[nodiscard]] std::expected<void, ice::Status> new_map(
        size_t key_size,
        size_t value_size,
        TF_MapHashFn hash_fn,
        TF_MapCompareFn compare_fn,
        int allow_duplicates
    ) noexcept
    {
        ice::Status status;
        m_ops->new_map(
            get_handle(),
            key_size,
            value_size,
            hash_fn,
            compare_fn,
            allow_duplicates,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    insert(const void* key, const void* value) noexcept
    {
        ice::Status status;
        m_ops->insert(get_handle(), key, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> find(const void* key) noexcept
    {
        ice::Status status;
        m_ops->find(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> erase(const void* key) noexcept
    {
        ice::Status status;
        m_ops->erase(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> contains(const void* key) noexcept
    {
        ice::Status status;
        m_ops->contains(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> size() noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_MapVisitor visitor, void* capture) noexcept
    {
        ice::Status status;
        m_ops->for_each(get_handle(), visitor, capture, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic

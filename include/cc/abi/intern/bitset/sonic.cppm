// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/bitset/bitset.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/bitset/bitset.h"

export module cc_abi_sonic_bitset;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Bitset : public ice::sonic::Runtime<Bitset, TF_BitSet>
{
public:
    explicit Bitset(TF_BitSet* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "bitset";

    [[nodiscard]] std::expected<void, ice::Status> new_bitset(size_t bit_count) noexcept
    {
        ice::Status status;
        m_ops->new_bitset(get_handle(), bit_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set(size_t index) noexcept
    {
        ice::Status status;
        m_ops->set(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> clear(size_t index) noexcept
    {
        ice::Status status;
        m_ops->clear(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> test(size_t index) noexcept
    {
        ice::Status status;
        m_ops->test(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> flip(size_t index) noexcept
    {
        ice::Status status;
        m_ops->flip(get_handle(), index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> count() noexcept
    {
        ice::Status status;
        m_ops->count(get_handle(), status.get_handle());

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

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic

// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/span/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/span/span.h"

export module cc_abi_sonic_span;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Span : public ice::sonic::Runtime<Span, TF_Span>
{
public:
    explicit Span(TF_Span* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "span";

    [[nodiscard]] std::expected<void, ice::Status>
    new_span(void* data, size_t count, size_t element_size) noexcept
    {
        ice::Status status;
        m_ops->new_span(get_handle(), data, count, element_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get(size_t index) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), index, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> data() noexcept
    {
        ice::Status status;
        m_ops->data(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> subspan(size_t offset, size_t count) noexcept
    {
        ice::Status status;
        m_ops->subspan(get_handle(), offset, count, status.get_handle());

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

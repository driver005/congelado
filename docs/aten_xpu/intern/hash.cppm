module;

#include "include/c/intern/hash.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:hash;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// 64-bit FNV-1a over bytes, and the boost style combine of two hashes.
class Hash : public ice::builder::Hash
{
public:
    Hash(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Hash{status_ops, string_ops},
        m_failure{string_ops}
    {
    }

    ~Hash() override = default;
    Hash(const Hash&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(Hash&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("Hash");
    }

    void hash_bytes(
        const void* data,
        std::size_t size,
        std::size_t& out_hash,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        out_hash = k_offset_basis;
        if (data == nullptr && size != 0) {
            m_failure.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "hash_bytes: null data with non zero size"
            );
            return;
        }

        const auto* bytes = static_cast<const unsigned char*>(data);
        for (std::size_t index = 0; index < size; ++index) {
            out_hash ^= bytes[index];
            out_hash *= k_prime;
        }
    }

    void hash_combine(
        std::size_t seed,
        std::size_t value,
        std::size_t& out_hash,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        out_hash = seed ^ (value + k_golden + (seed << 6) + (seed >> 2));
    }

private:
    static constexpr std::size_t k_offset_basis = 14'695'981'039'346'656'037ULL;
    static constexpr std::size_t k_prime = 1'099'511'628'211ULL;
    static constexpr std::size_t k_golden = 0x9E'37'79'B9'7F'4A'7C'15ULL;

    Failure m_failure;
};

} // namespace aten_xpu

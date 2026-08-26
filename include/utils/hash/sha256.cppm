module;

#include <openssl/evp.h>

export module utils_hash;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils {

class Sha256
{
public:
    /**
     * @brief Computes the SHA-256 digest of `data` and renders it as a lowercase hex string.
     * @param data the bytes to hash.
     * @return 64-character lowercase hex digest, or an empty string if OpenSSL's EVP digest
     * machinery fails to initialize/finalize — never the legitimate digest of any input (even
     * empty input hashes to a well-known non-empty 64-char digest), so an empty return is an
     * unambiguous failure signal to callers, not "hash of nothing".
     */
    [[nodiscard]] static std::string hash_hex(std::string_view data) noexcept
    {
        auto* ctx = EVP_MD_CTX_new();
        if (ctx == nullptr) {
            return "";
        }

        std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
        unsigned int digest_len = 0;
        bool ok = EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1 &&
                  EVP_DigestUpdate(ctx, data.data(), data.size()) == 1 &&
                  EVP_DigestFinal_ex(ctx, digest.data(), &digest_len) == 1;
        EVP_MD_CTX_free(ctx);
        if (!ok) {
            return "";
        }

        std::string hex;
        hex.reserve(static_cast<std::size_t>(digest_len) * 2);
        for (unsigned int i = 0; i < digest_len; ++i) {
            std::format_to(std::back_inserter(hex), "{:02x}", digest[i]);
        }
        return hex;
    }
};

} // namespace utils

#ifdef CONGELADO_TEST
namespace utils::tests {
using namespace boost::ut;

suite<"Sha256"> sha256_suite = [] {
    "matches the well-known digest of an empty input"_test = [] {
        expect(
            Sha256::hash_hex("") ==
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
        );
    };
    "matches the well-known digest of \"abc\""_test = [] {
        expect(
            Sha256::hash_hex("abc") ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        );
    };
    "digest is deterministic and 64 lowercase hex characters"_test = [] {
        auto digest = Sha256::hash_hex("congelado");
        expect(digest.size() == 64);
        expect(digest == Sha256::hash_hex("congelado"));
        expect(std::ranges::all_of(digest, [](char character) {
            return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f');
        }));
    };
};

} // namespace utils::tests
#endif

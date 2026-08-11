module;

#include <openssl/evp.h>

export module utils_hash;

import std;

export namespace utils {

class Sha256 {
  public:
    /**
     * @brief Computes the SHA-256 digest of `data` and renders it as a lowercase hex string.
     * @param data the bytes to hash.
     * @return 64-character lowercase hex digest, or an empty string if OpenSSL's EVP digest
     * machinery fails to initialize/finalize — never the legitimate digest of any input (even
     * empty input hashes to a well-known non-empty 64-char digest), so an empty return is an
     * unambiguous failure signal to callers, not "hash of nothing".
     */
    [[nodiscard]] static std::string hash_hex(std::string_view data) noexcept {
        auto *ctx = EVP_MD_CTX_new();
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

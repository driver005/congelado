module;

// Global module fragment: where you put non-modular includes
#include <openssl/err.h>
#include <openssl/ssl.h>

export module openssl;

// Export the specific functions or types you need
export using ::SSL_CTX;
export using ::SSL_new;
// export using ::SSL_library_init;

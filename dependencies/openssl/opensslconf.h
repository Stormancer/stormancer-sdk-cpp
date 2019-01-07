#ifndef OPENSSL_MULTIARCH_CONF_HEADER
#define OPENSSL_MULTIARCH_CONF_HEADER

#if defined(__i386__)
#include <openssl/opensslconf_i386.h>

#elif defined(__amd64__)
#include <openssl/opensslconf_x86_64.h>

#elif defined(__arm__)
#include <openssl/opensslconf_armv7.h>

#elif defined(__aarch64__)
#include <openssl/opensslconf_arm64.h>

#else
#error "Unknown architecture"

#endif
#endif // OPENSSL_MULTIARCH_CONF_HEADER

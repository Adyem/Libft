#include "ecc.hpp"
#include <sodium.h>

int ect_generate_keypair(unsigned char *public_key, unsigned char *private_key)
{
    if (sodium_init() < 0)
        return (-1);
    if (crypto_sign_keypair(public_key, private_key) != 0)
        return (-1);
    return (0);
}

int ect_compute_shared_secret(unsigned char *shared_secret,
                              const unsigned char *private_key,
                              const unsigned char *peer_public_key)
{
    if (sodium_init() < 0)
        return (-1);
    unsigned char curve_private[ECT_SHARED_SECRET_SIZE];
    unsigned char curve_public[ECT_SHARED_SECRET_SIZE];
    if (crypto_sign_ed25519_sk_to_curve25519(curve_private, private_key) != 0)
        return (-1);
    if (crypto_sign_ed25519_pk_to_curve25519(curve_public, peer_public_key) != 0)
        return (-1);
    if (crypto_scalarmult(shared_secret, curve_private, curve_public) != 0)
        return (-1);
    return (0);
}

int ect_sign(const unsigned char *private_key,
             const unsigned char *message,
             unsigned long long message_length,
             unsigned char *signature)
{
    if (sodium_init() < 0)
        return (-1);
    unsigned long long signature_length = 0;
    if (crypto_sign_detached(signature, &signature_length, message, message_length, private_key) != 0)
        return (-1);
    if (signature_length != ECT_SIGNATURE_SIZE)
        return (-1);
    return (0);
}

int ect_verify(const unsigned char *public_key,
               const unsigned char *message,
               unsigned long long message_length,
               const unsigned char *signature)
{
    if (sodium_init() < 0)
        return (-1);
    if (crypto_sign_verify_detached(signature, message, message_length, public_key) != 0)
        return (-1);
    return (0);
}

int ect_public_key_to_hex(const unsigned char *public_key, std::string &hex)
{
    if (sodium_init() < 0)
        return (-1);
    char buffer[ECT_PUBLIC_KEY_SIZE * 2 + 1];
    sodium_bin2hex(buffer, sizeof(buffer), public_key, ECT_PUBLIC_KEY_SIZE);
    hex.assign(buffer);
    return (0);
}

int ect_public_key_from_hex(const std::string &hex, unsigned char *public_key)
{
    if (sodium_init() < 0)
        return (-1);
    size_t binary_length = 0;
    if (sodium_hex2bin(public_key, ECT_PUBLIC_KEY_SIZE,
                       hex.c_str(), hex.size(),
                       NULL, &binary_length, NULL) != 0)
        return (-1);
    if (binary_length != ECT_PUBLIC_KEY_SIZE)
        return (-1);
    return (0);
}

int ect_private_key_to_hex(const unsigned char *private_key, std::string &hex)
{
    if (sodium_init() < 0)
        return (-1);
    char buffer[ECT_PRIVATE_KEY_SIZE * 2 + 1];
    sodium_bin2hex(buffer, sizeof(buffer), private_key, ECT_PRIVATE_KEY_SIZE);
    hex.assign(buffer);
    return (0);
}

int ect_private_key_from_hex(const std::string &hex, unsigned char *private_key)
{
    if (sodium_init() < 0)
        return (-1);
    size_t binary_length = 0;
    if (sodium_hex2bin(private_key, ECT_PRIVATE_KEY_SIZE,
                       hex.c_str(), hex.size(),
                       NULL, &binary_length, NULL) != 0)
        return (-1);
    if (binary_length != ECT_PRIVATE_KEY_SIZE)
        return (-1);
    return (0);
}

int ect_signature_to_hex(const unsigned char *signature, std::string &hex)
{
    if (sodium_init() < 0)
        return (-1);
    char buffer[ECT_SIGNATURE_SIZE * 2 + 1];
    sodium_bin2hex(buffer, sizeof(buffer), signature, ECT_SIGNATURE_SIZE);
    hex.assign(buffer);
    return (0);
}

int ect_signature_from_hex(const std::string &hex, unsigned char *signature)
{
    if (sodium_init() < 0)
        return (-1);
    size_t binary_length = 0;
    if (sodium_hex2bin(signature, ECT_SIGNATURE_SIZE,
                       hex.c_str(), hex.size(),
                       NULL, &binary_length, NULL) != 0)
        return (-1);
    if (binary_length != ECT_SIGNATURE_SIZE)
        return (-1);
    return (0);
}

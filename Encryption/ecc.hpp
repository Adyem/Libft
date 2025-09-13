#ifndef ECT_HPP
#define ECT_HPP

#include <cstddef>
#include <string>

constexpr std::size_t ECT_PUBLIC_KEY_SIZE = 32;
constexpr std::size_t ECT_PRIVATE_KEY_SIZE = 64;
constexpr std::size_t ECT_SIGNATURE_SIZE = 64;
constexpr std::size_t ECT_SHARED_SECRET_SIZE = 32;

int ect_generate_keypair(unsigned char *public_key, unsigned char *private_key);
int ect_compute_shared_secret(unsigned char *shared_secret,
                              const unsigned char *private_key,
                              const unsigned char *peer_public_key);
int ect_sign(const unsigned char *private_key,
             const unsigned char *message,
             unsigned long long message_length,
             unsigned char *signature);
int ect_verify(const unsigned char *public_key,
               const unsigned char *message,
               unsigned long long message_length,
               const unsigned char *signature);
int ect_public_key_to_hex(const unsigned char *public_key, std::string &hex);
int ect_public_key_from_hex(const std::string &hex, unsigned char *public_key);
int ect_private_key_to_hex(const unsigned char *private_key, std::string &hex);
int ect_private_key_from_hex(const std::string &hex, unsigned char *private_key);
int ect_signature_to_hex(const unsigned char *signature, std::string &hex);
int ect_signature_from_hex(const std::string &hex, unsigned char *signature);

#endif

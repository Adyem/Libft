#ifndef RSA_HPP
#define RSA_HPP

#include <stdint.h>

int        rsa_generate_key_pair(uint64_t *public_key, uint64_t *private_key, uint64_t *modulus, int bit_size);
uint64_t    rsa_encrypt(uint64_t message, uint64_t public_key, uint64_t modulus);
uint64_t    rsa_decrypt(uint64_t cipher, uint64_t private_key, uint64_t modulus);
void        rsa_set_force_mod_inverse_failure(bool enable);

#endif

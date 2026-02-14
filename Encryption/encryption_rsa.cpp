#include <stdint.h>
#include "../Errno/errno.hpp"
#include "../RNG/rng.hpp"
#include "rsa.hpp"

static bool g_force_mod_inverse_failure = false;

static uint64_t rsa_gcd(uint64_t first_value, uint64_t second_value)
{
    while (second_value != 0)
    {
        uint64_t temp_value = second_value;
        second_value = first_value % second_value;
        first_value = temp_value;
    }
    return (first_value);
}

static uint64_t rsa_mod_inverse(uint64_t value, uint64_t modulus)
{
    int64_t t_value = 0;
    int64_t new_t_value = 1;
    int64_t r_value = static_cast<int64_t>(modulus);
    int64_t new_r_value = static_cast<int64_t>(value);
    while (new_r_value != 0)
    {
        int64_t quotient_value = r_value / new_r_value;
        int64_t temp_t_value = t_value - quotient_value * new_t_value;
        t_value = new_t_value;
        new_t_value = temp_t_value;
        int64_t temp_r_value = r_value - quotient_value * new_r_value;
        r_value = new_r_value;
        new_r_value = temp_r_value;
    }
    if (r_value > 1)
        return (0);
    if (t_value < 0)
        t_value += modulus;
    return (static_cast<uint64_t>(t_value));
}

static bool rsa_is_prime(uint64_t value)
{
    if (value < 2)
        return (false);
    uint64_t divisor_value = 2;
    while (divisor_value * divisor_value <= value)
    {
        if (value % divisor_value == 0)
            return (false);
        divisor_value = divisor_value + 1;
    }
    return (true);
}

static uint64_t rsa_generate_prime(uint64_t limit)
{
    uint64_t prime_candidate = 0;
    while (prime_candidate < 2 || rsa_is_prime(prime_candidate) == false)
    {
        prime_candidate = static_cast<uint64_t>(ft_random_int()) % limit;
        if ((prime_candidate & 1) == 0)
            prime_candidate = prime_candidate + 1;
    }
    return (prime_candidate);
}

static uint64_t rsa_mod_pow(uint64_t base_value, uint64_t exponent_value, uint64_t modulus_value)
{
    uint64_t result_value = 1;
    base_value = base_value % modulus_value;
    while (exponent_value > 0)
    {
        if ((exponent_value & 1) == 1)
            result_value = (result_value * base_value) % modulus_value;
        exponent_value = exponent_value >> 1;
        base_value = (base_value * base_value) % modulus_value;
    }
    return (result_value);
}

int rsa_generate_key_pair(uint64_t *public_key, uint64_t *private_key, uint64_t *modulus, int bit_size)
{
    if (!public_key || !private_key || !modulus)
        return (1);
    uint64_t limit_value = 1ULL << (bit_size / 2);
    uint64_t prime_one = rsa_generate_prime(limit_value);
    uint64_t prime_two = rsa_generate_prime(limit_value);
    while (prime_two == prime_one)
        prime_two = rsa_generate_prime(limit_value);
    uint64_t phi_value = (prime_one - 1) * (prime_two - 1);
    *modulus = prime_one * prime_two;
    uint64_t public_exponent = 65537;
    if (rsa_gcd(public_exponent, phi_value) != 1)
    {
        public_exponent = 3;
        while (rsa_gcd(public_exponent, phi_value) != 1)
            public_exponent = public_exponent + 2;
    }
    uint64_t private_exponent;
    if (g_force_mod_inverse_failure == true)
        private_exponent = 0;
    else
        private_exponent = rsa_mod_inverse(public_exponent, phi_value);
    if (private_exponent == 0)
        return (1);
    *public_key = public_exponent;
    *private_key = private_exponent;
    return (0);
}

void rsa_set_force_mod_inverse_failure(bool enable)
{
    g_force_mod_inverse_failure = enable;
    return ;
}

uint64_t rsa_encrypt(uint64_t message, uint64_t public_key, uint64_t modulus)
{
    return (rsa_mod_pow(message, public_key, modulus));
}

uint64_t rsa_decrypt(uint64_t cipher, uint64_t private_key, uint64_t modulus)
{
    return (rsa_mod_pow(cipher, private_key, modulus));
}

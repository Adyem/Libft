#include "../../Encryption/rsa.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../RNG/rng_internal.hpp"

FT_TEST(test_rsa_generate_key_pair_null_output_sets_errno,
    "rsa_generate_key_pair rejects null outputs")
{
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    ft_errno = ER_SUCCESS;
    int result_value = rsa_generate_key_pair(nullptr, &private_key_value, &modulus_value, 32);
    FT_ASSERT_EQ(1, result_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_rsa_generate_key_pair_mod_inverse_failure_sets_errno,
    "rsa_generate_key_pair reports modular inverse failure")
{
    uint64_t public_key_value = 0;
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    ft_errno = ER_SUCCESS;
    rsa_set_force_mod_inverse_failure(true);
    int result_value = rsa_generate_key_pair(&public_key_value, &private_key_value, &modulus_value, 32);
    rsa_set_force_mod_inverse_failure(false);
    FT_ASSERT_EQ(1, result_value);
    FT_ASSERT_EQ(FT_ERR_TERMINATED, ft_errno);
    return (1);
}

FT_TEST(test_rsa_generate_key_pair_success_clears_errno,
    "rsa_generate_key_pair succeeds and clears errno")
{
    uint64_t public_key_value = 0;
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_seed_random_engine(1234u);
    int result_value = rsa_generate_key_pair(&public_key_value, &private_key_value, &modulus_value, 32);
    FT_ASSERT_EQ(0, result_value);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(public_key_value != 0);
    FT_ASSERT(private_key_value != 0);
    FT_ASSERT(modulus_value != 0);
    return (1);
}

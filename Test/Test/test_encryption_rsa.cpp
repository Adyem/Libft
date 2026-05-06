#include "../test_internal.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/RNG/rng_internal.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rsa_generate_key_pair_null_output_sets_errno)
{
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    int result_value = rsa_generate_key_pair(nullptr, &private_key_value, &modulus_value, 32);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result_value);
    return (1);
}

FT_TEST(test_rsa_generate_key_pair_mod_inverse_failure_sets_errno)
{
    uint64_t public_key_value = 0;
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    rsa_set_force_mod_inverse_failure(true);
    int result_value = rsa_generate_key_pair(&public_key_value, &private_key_value, &modulus_value, 32);
    rsa_set_force_mod_inverse_failure(false);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, result_value);
    return (1);
}

FT_TEST(test_rsa_generate_key_pair_success_clears_errno)
{
    uint64_t public_key_value = 0;
    uint64_t private_key_value = 0;
    uint64_t modulus_value = 0;
    ft_seed_random_engine(1234u);
    int result_value = rsa_generate_key_pair(&public_key_value, &private_key_value, &modulus_value, 32);
    FT_ASSERT_EQ(0, result_value);
    FT_ASSERT(public_key_value != 0);
    FT_ASSERT(private_key_value != 0);
    FT_ASSERT(modulus_value != 0);
    return (1);
}

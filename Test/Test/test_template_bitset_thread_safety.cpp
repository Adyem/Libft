#include "../test_internal.hpp"
#include "../../Template/bitset.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_bitset_thread_safety_guards_access,
        "ft_bitset mutex guards access and changes remain visible")
{
    ft_bitset bits(8);
    bool lock_acquired;

    FT_ASSERT_EQ(false, bits.is_thread_safe());
    FT_ASSERT_EQ(0, bits.enable_thread_safety());
    FT_ASSERT_EQ(true, bits.is_thread_safe());
    bits.set(3);
    lock_acquired = false;
    FT_ASSERT_EQ(0, bits.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(true, bits.test(3));
    bits.unlock(lock_acquired);
    bits.disable_thread_safety();
    FT_ASSERT_EQ(false, bits.is_thread_safe());
    return (1);
}

FT_TEST(test_bitset_thread_safety_disable_clears_mutex,
        "ft_bitset disabling thread safety removes the guard")
{
    ft_bitset bits(6);

    FT_ASSERT_EQ(0, bits.enable_thread_safety());
    FT_ASSERT_EQ(true, bits.is_thread_safe());
    bits.disable_thread_safety();
    FT_ASSERT_EQ(false, bits.is_thread_safe());
    return (1);
}

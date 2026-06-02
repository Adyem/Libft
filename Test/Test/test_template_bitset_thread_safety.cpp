#include "../test_internal.hpp"
#include "../../Modules/CPP_class/bitset.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_bitset_thread_safety_guards_access)
{
    ft_bitset bits(8);
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, bits.initialize(8));
    FT_ASSERT_EQ(FT_FALSE, bits.is_thread_safe());
    FT_ASSERT_EQ(0, bits.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, bits.is_thread_safe());
    bits.set(3);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, bits.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    FT_ASSERT_EQ(FT_TRUE, bits.test(3));
    bits.unlock(lock_acquired);
    bits.disable_thread_safety();
    FT_ASSERT_EQ(FT_FALSE, bits.is_thread_safe());
    return (1);
}

FT_TEST(test_bitset_thread_safety_disable_clears_mutex)
{
    ft_bitset bits(6);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, bits.initialize(6));
    FT_ASSERT_EQ(0, bits.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, bits.is_thread_safe());
    bits.disable_thread_safety();
    FT_ASSERT_EQ(FT_FALSE, bits.is_thread_safe());
    return (1);
}

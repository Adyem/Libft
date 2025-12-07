#include "../../Template/bitset.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_bitset_thread_safety_resets_errno,
        "ft_bitset installs optional mutex guard and resets errno to success")
{
    ft_bitset bits(8);
    bool lock_acquired;

    FT_ASSERT_EQ(false, bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, bits.enable_thread_safety());
    FT_ASSERT_EQ(true, bits.is_thread_safe_enabled());
    bits.set(3);
    ft_errno = FT_ERR_INVALID_OPERATION;
    lock_acquired = false;
    FT_ASSERT_EQ(0, bits.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    bits.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(true, bits.test(3));
    bits.disable_thread_safety();
    FT_ASSERT_EQ(false, bits.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_bitset_move_transfers_thread_safety,
        "ft_bitset move operations transfer thread safety state")
{
    ft_bitset original(4);

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());
    original.set(1);

    ft_bitset moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved.test(1));

    ft_bitset assigned(2);
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    assigned.set(0);
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, assigned.test(1));

    return (1);
}

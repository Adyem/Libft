#include "../test_internal.hpp"
#include "../../Template/iterator.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_iterator_thread_safety_sets_last_error_success,
        "Iterator installs optional mutex guards and reports success")
{
    int values[2] = {1, 2};
    Iterator<int> iterator(values);
    bool lock_acquired;

    FT_ASSERT_EQ(false, iterator.is_thread_safe());
    FT_ASSERT_EQ(0, iterator.enable_thread_safety());
    FT_ASSERT_EQ(true, iterator.is_thread_safe());
    lock_acquired = false;
    FT_ASSERT_EQ(0, iterator.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, Iterator<int>::last_operation_error());
    iterator.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, Iterator<int>::last_operation_error());
    iterator.disable_thread_safety();
    FT_ASSERT_EQ(false, iterator.is_thread_safe());
    return (1);
}

FT_TEST(test_iterator_thread_safety_reinitialized_manually,
        "Iterator manual reset preserves mutex behavior")
{
    int values[3] = {5, 10, 15};
    Iterator<int> original(values);
    Iterator<int> replica;

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe());
    FT_ASSERT_EQ(5, *original);

    FT_ASSERT_EQ(0, replica.initialize(values));
    FT_ASSERT_EQ(0, replica.enable_thread_safety());
    FT_ASSERT_EQ(true, replica.is_thread_safe());
    FT_ASSERT_EQ(5, *replica);

    FT_ASSERT_EQ(0, replica.destroy());
    FT_ASSERT_EQ(0, replica.initialize(values + 1));
    FT_ASSERT_EQ(0, replica.enable_thread_safety());
    FT_ASSERT_EQ(true, replica.is_thread_safe());
    FT_ASSERT_EQ(10, *replica);

    FT_ASSERT_EQ(0, original.disable_thread_safety());
    FT_ASSERT_EQ(false, original.is_thread_safe());
    FT_ASSERT_EQ(0, original.initialize(values + 2));
    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe());
    FT_ASSERT_EQ(15, *original);
    return (1);
}

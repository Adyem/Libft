#include "../test_internal.hpp"
#include "compatebility_system_test_hooks.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <cerrno>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_get_cpu_count_forced_success_sets_success)
{
    cmp_set_force_cpu_count_success(8);
    FT_ASSERT_EQ(8u, su_get_cpu_count());
    cmp_clear_force_cpu_count_result();
    return (1);
}

FT_TEST(test_su_get_cpu_count_forced_failure_errno)
{
    cmp_set_force_cpu_count_failure(EIO);
    FT_ASSERT_EQ(0u, su_get_cpu_count());
    cmp_clear_force_cpu_count_result();
    return (1);
}

FT_TEST(test_su_get_cpu_count_forced_failure_fallback)
{
    cmp_set_force_cpu_count_failure(0);
    FT_ASSERT_EQ(0u, su_get_cpu_count());
    cmp_clear_force_cpu_count_result();
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_success_sets_success)
{
    cmp_set_force_total_memory_success(123456ull);
    FT_ASSERT_EQ(123456ull, su_get_total_memory());
    cmp_clear_force_total_memory_result();
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_failure_errno)
{
    cmp_set_force_total_memory_failure(ENOMEM);
    FT_ASSERT_EQ(0ull, su_get_total_memory());
    cmp_clear_force_total_memory_result();
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_failure_fallback)
{
    cmp_set_force_total_memory_failure(0);
    FT_ASSERT_EQ(0ull, su_get_total_memory());
    cmp_clear_force_total_memory_result();
    return (1);
}

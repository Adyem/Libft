#include "../Compatebility/compatebility_system_test_hooks.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>

FT_TEST(test_su_get_cpu_count_forced_success_sets_success, "su_get_cpu_count forced success clears ft_errno")
{
    ft_errno = FT_EINVAL;
    cmp_set_force_cpu_count_success(8);
    FT_ASSERT_EQ(8u, su_get_cpu_count());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cmp_clear_force_cpu_count_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_su_get_cpu_count_forced_failure_errno, "su_get_cpu_count forced failure propagates errno")
{
    ft_errno = ER_SUCCESS;
    cmp_set_force_cpu_count_failure(EIO);
    FT_ASSERT_EQ(0u, su_get_cpu_count());
    FT_ASSERT_EQ(EIO + ERRNO_OFFSET, ft_errno);
    cmp_clear_force_cpu_count_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_su_get_cpu_count_forced_failure_fallback, "su_get_cpu_count forced failure falls back to FT_ETERM")
{
    ft_errno = ER_SUCCESS;
    cmp_set_force_cpu_count_failure(0);
    FT_ASSERT_EQ(0u, su_get_cpu_count());
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    cmp_clear_force_cpu_count_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_success_sets_success, "su_get_total_memory forced success clears ft_errno")
{
    ft_errno = FT_EINVAL;
    cmp_set_force_total_memory_success(123456ull);
    FT_ASSERT_EQ(123456ull, su_get_total_memory());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cmp_clear_force_total_memory_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_failure_errno, "su_get_total_memory forced failure propagates errno")
{
    ft_errno = ER_SUCCESS;
    cmp_set_force_total_memory_failure(ENOMEM);
    FT_ASSERT_EQ(0ull, su_get_total_memory());
    FT_ASSERT_EQ(ENOMEM + ERRNO_OFFSET, ft_errno);
    cmp_clear_force_total_memory_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_su_get_total_memory_forced_failure_fallback, "su_get_total_memory forced failure falls back to FT_ETERM")
{
    ft_errno = ER_SUCCESS;
    cmp_set_force_total_memory_failure(0);
    FT_ASSERT_EQ(0ull, su_get_total_memory());
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    cmp_clear_force_total_memory_result();
    ft_errno = ER_SUCCESS;
    return (1);
}

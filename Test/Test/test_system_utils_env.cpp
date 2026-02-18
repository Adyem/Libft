#include "../test_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "compatebility_system_test_hooks.hpp"
#include <cerrno>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_putenv_null_sets_ft_einval, "su_putenv null argument assigns FT_ERR_INVALID_ARGUMENT")
{
    FT_ASSERT_EQ(-1, su_putenv(ft_nullptr));
    return (1);
}

FT_TEST(test_su_putenv_success_clears_error, "su_putenv success clears ft_errno")
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV=1";
    cmp_clear_force_putenv_result();
    FT_ASSERT_EQ(0, su_putenv(environment_entry));
    cmp_unsetenv("FT_TEST_SU_PUTENV");
    return (1);
}

FT_TEST(test_su_putenv_forced_failure_overwrites_errno, "su_putenv forced failure sets errno from cmp_putenv")
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV_FAIL=1";
    cmp_set_force_putenv_result(-1, EACCES);
    FT_ASSERT_EQ(-1, su_putenv(environment_entry));
    cmp_clear_force_putenv_result();
    return (1);
}

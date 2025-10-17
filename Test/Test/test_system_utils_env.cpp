#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"
#include <cerrno>

FT_TEST(test_su_putenv_null_sets_ft_einval, "su_putenv null argument assigns FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_putenv(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_putenv_success_clears_error, "su_putenv success clears ft_errno")
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV=1";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    cmp_clear_force_putenv_result();
    FT_ASSERT_EQ(0, su_putenv(environment_entry));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cmp_unsetenv("FT_TEST_SU_PUTENV");
    return (1);
}

FT_TEST(test_su_putenv_forced_failure_propagates_error, "su_putenv forced failure keeps cmp_putenv errno")
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV_FAIL=1";

    ft_errno = ER_SUCCESS;
    cmp_set_force_putenv_result(-1, EACCES);
    FT_ASSERT_EQ(-1, su_putenv(environment_entry));
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, ft_errno);
    cmp_clear_force_putenv_result();
    return (1);
}

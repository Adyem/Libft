#include "../test_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Compatebility/compatebility_internal.hpp"
#include "compatebility_system_test_hooks.hpp"
#include <cerrno>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_putenv_null_sets_ft_einval)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_putenv(ft_nullptr));
    return (1);
}

FT_TEST(test_su_putenv_success_clears_error)
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV=1";
    cmp_clear_force_putenv_result();
    FT_ASSERT_EQ(0, su_putenv(environment_entry));
    cmp_unsetenv("FT_TEST_SU_PUTENV");
    return (1);
}

FT_TEST(test_su_putenv_forced_failure_overwrites_errno)
{
    static char environment_entry[] = "FT_TEST_SU_PUTENV_FAIL=1";
    cmp_set_force_putenv_result(-1, EACCES);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, su_putenv(environment_entry));
    cmp_clear_force_putenv_result();
    cmp_unsetenv("FT_TEST_SU_PUTENV_FAIL");
    return (1);
}

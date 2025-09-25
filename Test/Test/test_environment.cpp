#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_unsetenv_rejects_empty_name, "ft_unsetenv rejects empty names")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_unsetenv(""));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_unsetenv_rejects_equals_sign, "ft_unsetenv rejects names containing equals")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_unsetenv("INVALID=NAME"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_unsetenv_success_resets_errno, "ft_unsetenv clears ft_errno on success")
{
    const char *variable_name;

    variable_name = "LIBFT_TEST_UNSET_OK";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    ft_errno = FT_ETERM;
    FT_ASSERT_EQ(0, ft_unsetenv(variable_name));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_getenv(variable_name));
    return (1);
}


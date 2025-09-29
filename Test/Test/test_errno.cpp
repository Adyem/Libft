#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstring>

FT_TEST(test_ft_strerror_errno_message, "ft_strerror returns standard errno message")
{
    const char *expected_message;
    const char *actual_message;

    expected_message = strerror(EINVAL);
    actual_message = ft_strerror(EINVAL + ERRNO_OFFSET);
    FT_ASSERT(expected_message != NULL);
    FT_ASSERT(actual_message != NULL);
    FT_ASSERT_EQ(0, std::strcmp(expected_message, actual_message));
    return (1);
}

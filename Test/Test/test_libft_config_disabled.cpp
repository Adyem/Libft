#include "../test_internal.hpp"

#ifndef LIBFT_TEST_BUILD
#endif
#define LIBFT_ENABLE_ENVIRONMENT_HELPERS 0
#define LIBFT_ENABLE_FILE_IO_HELPERS 0
#define LIBFT_ENABLE_TIME_HELPERS 0
#define LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS 0
#define LIBFT_ENABLE_LOCALE_HELPERS 0
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

static_assert(LIBFT_HAS_ENVIRONMENT_HELPERS == 0, "environment helpers should be disabled");
static_assert(LIBFT_HAS_FILE_IO_HELPERS == 0, "file helpers should be disabled");
static_assert(LIBFT_HAS_TIME_HELPERS == 0, "time helpers should be disabled");
static_assert(LIBFT_HAS_BOUNDS_CHECKED_HELPERS == 0, "bounds helpers should be disabled");
static_assert(LIBFT_HAS_LOCALE_HELPERS == 0, "locale helpers should be disabled");

FT_TEST(test_libft_config_disables_environment_helpers, "Libft config disables environment helpers")
{
    FT_ASSERT_EQ(0, LIBFT_HAS_ENVIRONMENT_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_disables_file_helpers, "Libft config disables file helpers")
{
    FT_ASSERT_EQ(0, LIBFT_HAS_FILE_IO_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_disables_time_helpers, "Libft config disables time helpers")
{
    FT_ASSERT_EQ(0, LIBFT_HAS_TIME_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_disables_bounds_helpers, "Libft config disables bounds helpers")
{
    FT_ASSERT_EQ(0, LIBFT_HAS_BOUNDS_CHECKED_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_disables_locale_helpers, "Libft config disables locale helpers")
{
    FT_ASSERT_EQ(0, LIBFT_HAS_LOCALE_HELPERS);
    return (1);
}

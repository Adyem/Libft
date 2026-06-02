#include "../test_internal.hpp"
#include "../../Modules/Basic/config.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static_assert(LIBFT_HAS_ENVIRONMENT_HELPERS == 1, "environment helpers enabled by default");
static_assert(LIBFT_HAS_FILE_IO_HELPERS == 1, "file helpers enabled by default");
static_assert(LIBFT_HAS_TIME_HELPERS == 1, "time helpers enabled by default");
static_assert(LIBFT_HAS_BOUNDS_CHECKED_HELPERS == 1, "bounds helpers enabled by default");
static_assert(LIBFT_HAS_LOCALE_HELPERS == 1, "locale helpers enabled by default");

FT_TEST(test_libft_config_defaults_environment)
{
    FT_ASSERT_EQ(1, LIBFT_HAS_ENVIRONMENT_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_file_io)
{
    FT_ASSERT_EQ(1, LIBFT_HAS_FILE_IO_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_time)
{
    FT_ASSERT_EQ(1, LIBFT_HAS_TIME_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_bounds)
{
    FT_ASSERT_EQ(1, LIBFT_HAS_BOUNDS_CHECKED_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_locale)
{
    FT_ASSERT_EQ(1, LIBFT_HAS_LOCALE_HELPERS);
    return (1);
}

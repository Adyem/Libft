#include "../test_internal.hpp"
#include "../../Basic/config.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static_assert(LIBFT_HAS_ENVIRONMENT_HELPERS == 1, "environment helpers enabled by default");
static_assert(LIBFT_HAS_FILE_IO_HELPERS == 1, "file helpers enabled by default");
static_assert(LIBFT_HAS_TIME_HELPERS == 1, "time helpers enabled by default");
static_assert(LIBFT_HAS_BOUNDS_CHECKED_HELPERS == 1, "bounds helpers enabled by default");
static_assert(LIBFT_HAS_LOCALE_HELPERS == 1, "locale helpers enabled by default");

FT_TEST(test_libft_config_defaults_environment, "Libft config enables environment helpers by default")
{
    FT_ASSERT_EQ(1, LIBFT_HAS_ENVIRONMENT_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_file_io, "Libft config enables file helpers by default")
{
    FT_ASSERT_EQ(1, LIBFT_HAS_FILE_IO_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_time, "Libft config enables time helpers by default")
{
    FT_ASSERT_EQ(1, LIBFT_HAS_TIME_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_bounds, "Libft config enables bounds helpers by default")
{
    FT_ASSERT_EQ(1, LIBFT_HAS_BOUNDS_CHECKED_HELPERS);
    return (1);
}

FT_TEST(test_libft_config_defaults_locale, "Libft config enables locale helpers by default")
{
    FT_ASSERT_EQ(1, LIBFT_HAS_LOCALE_HELPERS);
    return (1);
}

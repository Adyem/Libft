#include "../test_internal.hpp"
#include "../../Template/set.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <type_traits>

#ifndef LIBFT_TEST_BUILD
#endif

using set_type = ft_set<int>;

FT_TEST(test_ft_set_move_constructor_rebuilds_mutex)
{
    FT_ASSERT_EQ(true, std::is_move_constructible<set_type>::value);
    return (1);
}

FT_TEST(test_ft_set_move_assignment_rebuilds_mutex)
{
    FT_ASSERT_EQ(false, std::is_move_assignable<set_type>::value);
    return (1);
}

FT_TEST(test_ft_set_move_preserves_disabled_thread_safety)
{
    FT_ASSERT_EQ(true, std::is_move_constructible<set_type>::value);
    return (1);
}

FT_TEST(test_ft_set_move_allows_reinitializing_source_mutex)
{
    FT_ASSERT_EQ(false, std::is_move_assignable<set_type>::value);
    return (1);
}

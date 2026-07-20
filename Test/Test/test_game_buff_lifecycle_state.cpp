#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_buff_get_modifier3(game_buff &value)
{
    (void)value.get_modifier3();
    return ;
}
static void game_buff_set_modifier3(game_buff &value)
{
    value.set_modifier3(1);
    return ;
}
static void game_buff_add_modifier3(game_buff &value)
{
    value.add_modifier3(1);
    return ;
}
static void game_buff_sub_modifier3(game_buff &value)
{
    value.sub_modifier3(1);
    return ;
}
static void game_buff_get_modifier4(game_buff &value)
{
    (void)value.get_modifier4();
    return ;
}
static void game_buff_set_modifier4(game_buff &value)
{
    value.set_modifier4(1);
    return ;
}
static void game_buff_add_modifier4(game_buff &value)
{
    value.add_modifier4(1);
    return ;
}
static void game_buff_sub_modifier4(game_buff &value)
{
    value.sub_modifier4(1);
    return ;
}
static void game_buff_get_error(game_buff &value)
{
    (void)value.get_error();
    return ;
}
static void game_buff_get_error_str(game_buff &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_buff_get_modifier3_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier3));
    return (1);
}
FT_TEST(test_game_buff_set_modifier3_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier3));
    return (1);
}
FT_TEST(test_game_buff_add_modifier3_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier3));
    return (1);
}
FT_TEST(test_game_buff_sub_modifier3_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier3));
    return (1);
}
FT_TEST(test_game_buff_get_modifier4_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier4));
    return (1);
}
FT_TEST(test_game_buff_set_modifier4_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier4));
    return (1);
}
FT_TEST(test_game_buff_add_modifier4_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier4));
    return (1);
}
FT_TEST(test_game_buff_sub_modifier4_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier4));
    return (1);
}
FT_TEST(test_game_buff_get_error_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_error));
    return (1);
}
FT_TEST(test_game_buff_get_error_str_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_error_str));
    return (1);
}

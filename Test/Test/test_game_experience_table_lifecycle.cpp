#include "../test_internal.hpp"

#include "../../Modules/Game/game_experience_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_experience_table_thread_safe_lifecycle)
{
    game_experience_table table;
    const int32_t levels[] = {10, 30, 60};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(0, table.get_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.set_levels(levels, 3));
    FT_ASSERT_EQ(3, table.get_count());
    FT_ASSERT_EQ(10, table.get_value(0));
    FT_ASSERT_EQ(30, table.get_value(1));
    FT_ASSERT_EQ(60, table.get_value(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.resize(5));
    FT_ASSERT_EQ(5, table.get_count());
    table.set_value(3, 110);
    table.set_value(4, 170);
    FT_ASSERT_EQ(110, table.get_value(3));
    FT_ASSERT_EQ(170, table.get_value(4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT(table.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    return (1);
}

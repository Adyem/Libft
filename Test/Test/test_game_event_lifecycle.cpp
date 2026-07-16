#include "../test_internal.hpp"

#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_event_thread_safe_lifecycle)
{
    game_event event;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, event.initialize());
    FT_ASSERT_EQ(0, event.get_id());
    FT_ASSERT_EQ(0, event.get_duration());
    FT_ASSERT_EQ(0, event.get_modifier1());
    FT_ASSERT_EQ(0, event.get_modifier2());
    FT_ASSERT_EQ(0, event.get_modifier3());
    FT_ASSERT_EQ(0, event.get_modifier4());
    event.set_id(44);
    event.set_duration(12);
    event.set_modifier1(1);
    event.set_modifier2(2);
    event.set_modifier3(3);
    event.set_modifier4(4);
    FT_ASSERT_EQ(44, event.get_id());
    FT_ASSERT_EQ(12, event.get_duration());
    FT_ASSERT_EQ(1, event.get_modifier1());
    FT_ASSERT_EQ(2, event.get_modifier2());
    FT_ASSERT_EQ(3, event.get_modifier3());
    FT_ASSERT_EQ(4, event.get_modifier4());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, event.enable_thread_safety());
    FT_ASSERT(event.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, event.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, event.destroy());
    return (1);
}

#include "../test_internal.hpp"
#include "../../Game/game_hooks.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstdint>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_hooks_reset_clears_callbacks, "Game: hooks reset clears registered callbacks")
{
    ft_game_hooks hooks;
    ft_character character;
    ft_item item;
    ft_world world;
    ft_event event;
    int crafted_invocations;
    int damaged_invocations;
    int event_invocations;

    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    hooks.set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        return ;
    }));
    hooks.set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)>([&damaged_invocations](ft_character &character_ref, int damage, uint8_t type)
    {
        (void)character_ref;
        (void)damage;
        (void)type;
        damaged_invocations = damaged_invocations + 1;
        return ;
    }));
    hooks.set_on_event_triggered(ft_function<void(ft_world&, ft_event&)>([&event_invocations](ft_world &world_ref, ft_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        event_invocations = event_invocations + 1;
        return ;
    }));
    hooks.reset();
    hooks.invoke_on_item_crafted(character, item);
    hooks.invoke_on_character_damaged(character, 7, static_cast<uint8_t>(3));
    hooks.invoke_on_event_triggered(world, event);
    FT_ASSERT_EQ(0, crafted_invocations);
    FT_ASSERT_EQ(0, damaged_invocations);
    FT_ASSERT_EQ(0, event_invocations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.get_error());
    return (1);
}

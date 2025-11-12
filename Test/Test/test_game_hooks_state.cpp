#include "../../Game/game_state.hpp"
#include "../../Game/game_hooks.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstdint>
#include <new>

FT_TEST(test_game_state_dispatches_registered_hooks, "Game: game state dispatch forwards callbacks")
{
    ft_game_state state;
    ft_sharedptr<ft_game_hooks> hooks;
    ft_character character;
    ft_item item;
    ft_world world;
    ft_event event;
    int crafted_invocations;
    int damaged_invocations;
    int event_invocations;

    hooks = ft_sharedptr<ft_game_hooks>(new (std::nothrow) ft_game_hooks());
    FT_ASSERT(static_cast<bool>(hooks));
    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    hooks->set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        return ;
    }));
    hooks->set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)>([&damaged_invocations](ft_character &character_ref, int damage, uint8_t type)
    {
        (void)character_ref;
        (void)damage;
        (void)type;
        damaged_invocations = damaged_invocations + 1;
        return ;
    }));
    hooks->set_on_event_triggered(ft_function<void(ft_world&, ft_event&)>([&event_invocations](ft_world &world_ref, ft_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        event_invocations = event_invocations + 1;
        return ;
    }));
    state.set_hooks(hooks);
    FT_ASSERT_EQ(ER_SUCCESS, state.get_error());
    state.dispatch_item_crafted(character, item);
    state.dispatch_character_damaged(character, 9, static_cast<uint8_t>(1));
    state.dispatch_event_triggered(world, event);
    FT_ASSERT_EQ(1, crafted_invocations);
    FT_ASSERT_EQ(1, damaged_invocations);
    FT_ASSERT_EQ(1, event_invocations);
    FT_ASSERT_EQ(ER_SUCCESS, hooks->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, state.get_error());
    return (1);
}

FT_TEST(test_game_state_reset_hooks_clears_callbacks, "Game: game state reset clears callbacks")
{
    ft_game_state state;
    ft_sharedptr<ft_game_hooks> hooks;
    ft_character character;
    ft_item item;
    ft_world world;
    ft_event event;
    int crafted_invocations;
    int damaged_invocations;
    int event_invocations;

    hooks = ft_sharedptr<ft_game_hooks>(new (std::nothrow) ft_game_hooks());
    FT_ASSERT(static_cast<bool>(hooks));
    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    hooks->set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        return ;
    }));
    hooks->set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)>([&damaged_invocations](ft_character &character_ref, int damage, uint8_t type)
    {
        (void)character_ref;
        (void)damage;
        (void)type;
        damaged_invocations = damaged_invocations + 1;
        return ;
    }));
    hooks->set_on_event_triggered(ft_function<void(ft_world&, ft_event&)>([&event_invocations](ft_world &world_ref, ft_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        event_invocations = event_invocations + 1;
        return ;
    }));
    state.set_hooks(hooks);
    state.dispatch_item_crafted(character, item);
    state.dispatch_character_damaged(character, 12, static_cast<uint8_t>(4));
    state.dispatch_event_triggered(world, event);
    FT_ASSERT_EQ(1, crafted_invocations);
    FT_ASSERT_EQ(1, damaged_invocations);
    FT_ASSERT_EQ(1, event_invocations);
    state.reset_hooks();
    FT_ASSERT_EQ(ER_SUCCESS, state.get_error());
    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    state.dispatch_item_crafted(character, item);
    state.dispatch_character_damaged(character, 6, static_cast<uint8_t>(3));
    state.dispatch_event_triggered(world, event);
    FT_ASSERT_EQ(0, crafted_invocations);
    FT_ASSERT_EQ(0, damaged_invocations);
    FT_ASSERT_EQ(0, event_invocations);
    FT_ASSERT_EQ(ER_SUCCESS, hooks->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, state.get_error());
    return (1);
}

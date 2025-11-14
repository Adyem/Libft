#include "../../Game/game_hooks.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstdint>

FT_TEST(test_game_hooks_invoke_callbacks, "Game: hooks dispatch registered callbacks")
{
    ft_game_hooks hooks;
    ft_character character;
    ft_item item;
    ft_world world;
    ft_event event;
    int crafted_invocations;
    int damaged_invocations;
    int event_invocations;
    int invocation_counter;
    int high_priority_step;
    int legacy_step;
    int low_priority_step;

    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    invocation_counter = 0;
    high_priority_step = 0;
    legacy_step = 0;
    low_priority_step = 0;
    hooks.set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations, &invocation_counter, &legacy_step](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        invocation_counter = invocation_counter + 1;
        legacy_step = invocation_counter;
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
    ft_game_hook_metadata high_metadata;
    ft_game_hook_metadata low_metadata;

    high_metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    high_metadata.listener_name = "test.high";
    high_metadata.description = "High priority test listener";
    high_metadata.argument_contract = "ft_character&,ft_item&";
    hooks.register_listener(high_metadata, 1500, ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)>([&invocation_counter, &high_priority_step](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        invocation_counter = invocation_counter + 1;
        high_priority_step = invocation_counter;
        return ;
    })));
    low_metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    low_metadata.listener_name = "test.low";
    low_metadata.description = "Low priority test listener";
    low_metadata.argument_contract = "ft_character&,ft_item&";
    hooks.register_listener(low_metadata, 500, ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)>([&invocation_counter, &low_priority_step](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        invocation_counter = invocation_counter + 1;
        low_priority_step = invocation_counter;
        return ;
    })));
    hooks.invoke_on_item_crafted(character, item);
    hooks.invoke_on_character_damaged(character, 5, static_cast<uint8_t>(2));
    hooks.invoke_on_event_triggered(world, event);
    FT_ASSERT_EQ(1, crafted_invocations);
    FT_ASSERT_EQ(1, damaged_invocations);
    FT_ASSERT_EQ(1, event_invocations);
    FT_ASSERT_EQ(1, high_priority_step);
    FT_ASSERT_EQ(2, legacy_step);
    FT_ASSERT_EQ(3, low_priority_step);
    FT_ASSERT_EQ(ER_SUCCESS, hooks.get_error());
    return (1);
}

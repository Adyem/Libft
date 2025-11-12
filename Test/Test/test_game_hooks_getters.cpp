#include "../../Game/game_hooks.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_hooks_getters_clone_callbacks, "Game: hooks getters provide callable copies")
{
    ft_game_hooks hooks;
    ft_character character;
    ft_item item;
    int crafted_invocations;

    crafted_invocations = 0;
    hooks.set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        return ;
    }));
    ft_function<void(ft_character&, ft_item&)> callback_copy;

    callback_copy = hooks.get_on_item_crafted();
    FT_ASSERT(static_cast<bool>(callback_copy));
    callback_copy(character, item);
    hooks.invoke_on_item_crafted(character, item);
    FT_ASSERT_EQ(2, crafted_invocations);
    FT_ASSERT_EQ(ER_SUCCESS, hooks.get_error());
    return (1);
}

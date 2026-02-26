#include "../test_internal.hpp"
#include "../../Game/game_hooks.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_hooks_getters_clone_callbacks, "Game: hooks getters provide callable copies")
{
    ft_game_hooks hooks;
    ft_character character;
    ft_item item;
    int crafted_invocations;
    ft_game_hook_metadata metadata;

    crafted_invocations = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    hooks.set_on_item_crafted(ft_function<void(ft_character&, ft_item&)>([&crafted_invocations](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        return ;
    }));
    metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    metadata.listener_name = "getter.custom";
    metadata.description = "Metadata test listener";
    metadata.argument_contract = "ft_character&,ft_item&";
    hooks.register_listener(metadata, 900, ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)>([](ft_character &character_ref, ft_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        return ;
    })));
    ft_function<void(ft_character&, ft_item&)> callback_copy;

    callback_copy = hooks.get_on_item_crafted();
    FT_ASSERT(static_cast<bool>(callback_copy));
    callback_copy(character, item);
    hooks.invoke_on_item_crafted(character, item);
    FT_ASSERT_EQ(2, crafted_invocations);
    ft_vector<ft_game_hook_metadata> catalog_after = hooks.get_catalog_metadata();
    ft_string identifier_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, identifier_string.initialize(ft_game_hook_item_crafted_identifier));
    ft_vector<ft_game_hook_metadata> scoped_catalog_after = hooks.get_catalog_metadata_for(identifier_string);
    FT_ASSERT_EQ(2, catalog_after.size());
    FT_ASSERT_EQ(2, scoped_catalog_after.size());
    ft_string listener_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, listener_name.initialize("getter.custom"));
    hooks.unregister_listener(identifier_string, listener_name);
    ft_vector<ft_game_hook_metadata> scoped_after = hooks.get_catalog_metadata_for(identifier_string);
    FT_ASSERT_EQ(1, scoped_after.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.get_error());
    return (1);
}

#include "../test_internal.hpp"
#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_equipment_equip)
{
    game_character hero;
    ft_sharedptr<game_item> helm(new game_item());

    FT_ASSERT(helm.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hero.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, helm->initialize());
    helm->set_item_id(1);
    helm->set_modifier1_id(1);
    helm->set_modifier1_value(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hero.equip_item(EQUIP_HEAD, helm));
    FT_ASSERT_EQ(5, hero.get_physical_armor());
    ft_sharedptr<game_item> found = hero.get_equipped_item(EQUIP_HEAD);
    FT_ASSERT(found.get() != ft_nullptr);
    return (1);
}

FT_TEST(test_game_equipment_unequip)
{
    game_character hero;
    ft_sharedptr<game_item> helm(new game_item());

    FT_ASSERT(helm.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hero.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, helm->initialize());
    helm->set_item_id(1);
    helm->set_modifier1_id(1);
    helm->set_modifier1_value(5);
    hero.equip_item(EQUIP_HEAD, helm);
    hero.unequip_item(EQUIP_HEAD);
    FT_ASSERT_EQ(0, hero.get_physical_armor());
    FT_ASSERT(hero.get_equipped_item(EQUIP_HEAD).get() == ft_nullptr);
    return (1);
}

FT_TEST(test_game_equipment_thread_safety_toggle)
{
    game_equipment equipment;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.initialize());
    FT_ASSERT_EQ(false, equipment.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.enable_thread_safety());
    FT_ASSERT_EQ(true, equipment.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.disable_thread_safety());
    FT_ASSERT_EQ(false, equipment.is_thread_safe());
    return (1);
}

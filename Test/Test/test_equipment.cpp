#include "../test_internal.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_equipment_equip, "equip item increases stats")
{
    ft_character hero;
    ft_sharedptr<ft_item> helm(new ft_item());

    FT_ASSERT(helm.get() != ft_nullptr);
    helm->set_item_id(1);
    helm->set_modifier1_id(1);
    helm->set_modifier1_value(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hero.equip_item(EQUIP_HEAD, helm));
    FT_ASSERT_EQ(5, hero.get_physical_armor());
    ft_sharedptr<ft_item> found = hero.get_equipped_item(EQUIP_HEAD);
    FT_ASSERT(found.get() != ft_nullptr);
    return (1);
}

FT_TEST(test_equipment_unequip, "unequip removes stats")
{
    ft_character hero;
    ft_sharedptr<ft_item> helm(new ft_item());

    FT_ASSERT(helm.get() != ft_nullptr);
    helm->set_item_id(1);
    helm->set_modifier1_id(1);
    helm->set_modifier1_value(5);
    hero.equip_item(EQUIP_HEAD, helm);
    hero.unequip_item(EQUIP_HEAD);
    FT_ASSERT_EQ(0, hero.get_physical_armor());
    FT_ASSERT(hero.get_equipped_item(EQUIP_HEAD).get() == ft_nullptr);
    return (1);
}

FT_TEST(test_equipment_thread_safety_toggle, "equipment thread safety toggles explicitly")
{
    ft_equipment equipment;

    FT_ASSERT_EQ(false, equipment.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.enable_thread_safety());
    FT_ASSERT_EQ(true, equipment.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.disable_thread_safety());
    FT_ASSERT_EQ(false, equipment.is_thread_safe());
    return (1);
}

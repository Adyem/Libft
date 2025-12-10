#include "../../Game/game_character.hpp"
#include "../../Game/game_item.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_equipment_equip, "equip item increases stats")
{
    ft_character hero;
    ft_item helm;
    helm.set_item_id(1);
    helm.set_modifier1_id(1);
    helm.set_modifier1_value(5);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, hero.equip_item(EQUIP_HEAD, helm));
    FT_ASSERT_EQ(5, hero.get_physical_armor());
    ft_sharedptr<ft_item> found = hero.get_equipped_item(EQUIP_HEAD);
    FT_ASSERT(found.get() != ft_nullptr);
    return (1);
}

FT_TEST(test_equipment_unequip, "unequip removes stats")
{
    ft_character hero;
    ft_item helm;
    helm.set_item_id(1);
    helm.set_modifier1_id(1);
    helm.set_modifier1_value(5);
    hero.equip_item(EQUIP_HEAD, helm);
    hero.unequip_item(EQUIP_HEAD);
    FT_ASSERT_EQ(0, hero.get_physical_armor());
    FT_ASSERT(hero.get_equipped_item(EQUIP_HEAD).get() == ft_nullptr);
    return (1);
}

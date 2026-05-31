#include "../test_internal.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_equipment.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_buff_invalid_duration_sets_errno)
{
    game_buff buff;
    int duration_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buff.initialize());

    buff.set_duration(10);
    buff.sub_duration(-3);
    duration_value = buff.get_duration();
    FT_ASSERT_EQ(10, duration_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buff.get_error());
    return (1);
}

FT_TEST(test_game_debuff_invalid_duration_sets_errno)
{
    game_debuff debuff;
    int duration_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, debuff.initialize());

    debuff.set_duration(6);
    debuff.add_duration(-4);
    duration_value = debuff.get_duration();
    FT_ASSERT_EQ(6, duration_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, debuff.get_error());
    return (1);
}

FT_TEST(test_game_skill_invalid_cooldown_sets_errno)
{
    game_skill skill;
    int cooldown_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.initialize());

    skill.set_cooldown(5);
    skill.sub_cooldown(9);
    cooldown_value = skill.get_cooldown();
    FT_ASSERT_EQ(-4, cooldown_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());
    return (1);
}

FT_TEST(test_game_equipment_invalid_slot_sets_errno)
{
    game_equipment equipment;
    ft_sharedptr<game_item> item;
    int result;

    item = ft_sharedptr<game_item>(new game_item());
    FT_ASSERT(item.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    item->set_item_id(42);
    result = equipment.equip(99, item);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(result, equipment.get_error());
    FT_ASSERT(equipment.get_item(EQUIP_HEAD)->get() == ft_nullptr);
    return (1);
}

FT_TEST(test_game_equipment_invalid_item_sets_errno)
{
    game_equipment equipment;
    ft_sharedptr<game_item> item;
    int result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, equipment.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    result = equipment.equip(EQUIP_HEAD, item);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(result, equipment.get_error());
    FT_ASSERT(equipment.get_item(EQUIP_HEAD)->get() == ft_nullptr);
    return (1);
}

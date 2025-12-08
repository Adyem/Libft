#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_equipment.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_skill.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_buff_invalid_duration_sets_errno,
        "ft_buff rejects negative duration and updates errno")
{
    ft_buff buff;
    int entry_errno;
    int duration_value;

    buff.set_duration(10);
    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = entry_errno;
    buff.sub_duration(-3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, buff.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    duration_value = buff.get_duration();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(10, duration_value);
    return (1);
}

FT_TEST(test_game_debuff_invalid_duration_sets_errno,
        "ft_debuff rejects negative duration and updates errno")
{
    ft_debuff debuff;
    int entry_errno;
    int duration_value;

    debuff.set_duration(6);
    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = entry_errno;
    debuff.add_duration(-4);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, debuff.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    duration_value = debuff.get_duration();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(6, duration_value);
    return (1);
}

FT_TEST(test_game_skill_invalid_cooldown_sets_errno,
        "ft_skill rejects subtracting excessive cooldown and updates errno")
{
    ft_skill skill;
    int entry_errno;
    int cooldown_value;

    skill.set_cooldown(5);
    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = entry_errno;
    skill.sub_cooldown(9);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, skill.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    cooldown_value = skill.get_cooldown();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(5, cooldown_value);
    return (1);
}

FT_TEST(test_game_equipment_invalid_slot_sets_errno,
        "ft_equipment rejects unknown slot assignments and updates errno")
{
    ft_equipment equipment;
    ft_sharedptr<ft_item> item;
    int entry_errno;
    int result;

    item = ft_sharedptr<ft_item>(new ft_item());
    FT_ASSERT(item.get() != ft_nullptr);
    item->set_item_id(42);
    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = entry_errno;
    result = equipment.equip(99, item);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, result);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, equipment.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    ft_errno = entry_errno;
    FT_ASSERT(equipment.get_item(EQUIP_HEAD).get() == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    return (1);
}

FT_TEST(test_game_equipment_invalid_item_sets_errno,
        "ft_equipment rejects null items and updates errno")
{
    ft_equipment equipment;
    ft_sharedptr<ft_item> item;
    int entry_errno;
    int result;

    item = ft_sharedptr<ft_item>();
    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = entry_errno;
    result = equipment.equip(EQUIP_HEAD, item);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, result);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, equipment.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    ft_errno = entry_errno;
    FT_ASSERT(equipment.get_item(EQUIP_HEAD).get() == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    return (1);
}

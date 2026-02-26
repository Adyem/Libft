#include "../test_internal.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_equipment.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include "../../JSon/json.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

json_group *serialize_inventory(const ft_inventory &inventory);
json_group *serialize_equipment(const ft_character &character);
json_group *serialize_quest(const ft_quest &quest);
json_group *serialize_character(const ft_character &character);
int deserialize_inventory(ft_inventory &inventory, json_group *group);

FT_TEST(test_serialize_inventory_allocation_failure_sets_errno, "serialize_inventory reports allocation failure")
{
    ft_inventory inventory;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    cma_set_alloc_limit(1);
    json_group *group = serialize_inventory(inventory);
    cma_set_alloc_limit(0);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_serialize_inventory_null_item_sets_errno, "serialize_inventory reports null item state")
{
    ft_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());

    inventory.get_items().insert(0, ft_sharedptr<ft_item>());
    json_group *group = serialize_inventory(inventory);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_serialize_inventory_success_clears_errno, "serialize_inventory clears errno on success")
{
    ft_inventory inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.initialize(1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    ft_sharedptr<ft_item> item(new ft_item());

    item->set_item_id(42);
    item->set_max_stack(5);
    item->set_stack_size(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    json_group *group = serialize_inventory(inventory);
    FT_ASSERT(group != ft_nullptr);
    json_free_groups(group);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_serialize_equipment_allocation_failure_sets_errno, "serialize_equipment reports allocation failure")
{
    ft_character character;

    cma_set_alloc_limit(1);
    json_group *group = serialize_equipment(character);
    cma_set_alloc_limit(0);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    return (1);
}

FT_TEST(test_serialize_equipment_success_clears_errno, "serialize_equipment clears errno on success")
{
    ft_character character;
    ft_sharedptr<ft_item> weapon(new ft_item());

    weapon->set_item_id(7);
    weapon->set_max_stack(1);
    weapon->set_stack_size(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.equip_item(EQUIP_WEAPON, weapon));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    json_group *group = serialize_equipment(character);
    FT_ASSERT(group != ft_nullptr);
    json_free_groups(group);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    return (1);
}

FT_TEST(test_serialize_quest_allocation_failure_sets_errno, "serialize_quest reports allocation failure")
{
    ft_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    cma_set_alloc_limit(1);
    json_group *group = serialize_quest(quest);
    cma_set_alloc_limit(0);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_serialize_quest_null_reward_sets_errno, "serialize_quest reports null reward state")
{
    ft_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    quest.get_reward_items().push_back(ft_sharedptr<ft_item>());
    json_group *group = serialize_quest(quest);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_serialize_quest_success_clears_errno, "serialize_quest clears errno on success")
{
    ft_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    ft_sharedptr<ft_item> reward(new ft_item());

    reward->set_item_id(9);
    reward->set_max_stack(3);
    reward->set_stack_size(1);
    quest.get_reward_items().push_back(reward);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    json_group *group = serialize_quest(quest);
    FT_ASSERT(group != ft_nullptr);
    json_free_groups(group);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_serialize_character_allocation_failure_sets_errno, "serialize_character reports allocation failure")
{
    ft_character character;

    cma_set_alloc_limit(1);
    json_group *group = serialize_character(character);
    cma_set_alloc_limit(0);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    return (1);
}

FT_TEST(test_serialize_character_skill_error_sets_errno, "serialize_character reports skill container error")
{
    cma_set_alloc_limit(1);
    ft_character broken_character;
    cma_set_alloc_limit(0);

    json_group *group = serialize_character(broken_character);
    FT_ASSERT(group == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, broken_character.get_error());
    return (1);
}

FT_TEST(test_serialize_character_success_clears_errno, "serialize_character clears errno on success")
{
    ft_character character;
    ft_skill skill;

    skill.set_id(3);
    skill.set_level(2);
    skill.set_cooldown(5);
    skill.set_modifier1(1);
    skill.set_modifier2(2);
    skill.set_modifier3(3);
    skill.set_modifier4(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.add_skill(skill));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    json_group *group = serialize_character(character);
    FT_ASSERT(group != ft_nullptr);
    json_free_groups(group);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.get_error());
    return (1);
}

FT_TEST(test_deserialize_inventory_failure_then_success_updates_errno, "deserialize_inventory updates errno after failure and success")
{
    ft_inventory inventory;
    json_group *group = json_create_json_group("inventory");

    FT_ASSERT(group != ft_nullptr);
    json_item *item = json_create_item("capacity", 2);

    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("weight_limit", 50);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("current_weight", 10);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("used_slots", 1);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_count", 1);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_stack_size", 1);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_id", 7);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_width", 1);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_height", 1);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod1_id", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod1_value", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod2_id", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod2_value", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod3_id", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod3_value", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod4_id", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    item = json_create_item("item_0_mod4_value", 0);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, deserialize_inventory(inventory, group));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, inventory.get_error());
    item = json_create_item("item_0_max_stack", 5);
    FT_ASSERT(item != ft_nullptr);
    json_add_item_to_group(group, item);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deserialize_inventory(inventory, group));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    json_free_groups(group);
    return (1);
}

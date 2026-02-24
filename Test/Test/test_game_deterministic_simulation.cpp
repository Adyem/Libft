#include "../test_internal.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_crafting.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_deterministic_simulation_scenarios,
    "Game deterministic simulation across combat, crafting, quests, and events")
{
    ft_sharedptr<ft_world> world_instance(new (std::nothrow) ft_world());
    FT_ASSERT(world_instance.get() != ft_nullptr);
    ft_character hero_character;
    ft_character enemy_character;
    hero_character.set_damage_rule(FT_DAMAGE_RULE_FLAT);
    enemy_character.set_damage_rule(FT_DAMAGE_RULE_FLAT);
    hero_character.set_hit_points(52);
    enemy_character.set_hit_points(40);
    enemy_character.set_physical_armor(0);
    ft_inventory hero_inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hero_inventory.initialize(6, 0));

    ft_sharedptr<ft_item> oak_plank(new (std::nothrow) ft_item());
    FT_ASSERT(oak_plank.get() != ft_nullptr);
    oak_plank->set_item_id(101);
    oak_plank->set_max_stack(10);
    oak_plank->set_stack_size(3);
    oak_plank->set_rarity(1);

    ft_sharedptr<ft_item> iron_ingot(new (std::nothrow) ft_item());
    FT_ASSERT(iron_ingot.get() != ft_nullptr);
    iron_ingot->set_item_id(102);
    iron_ingot->set_max_stack(10);
    iron_ingot->set_stack_size(1);
    iron_ingot->set_rarity(2);

    FT_ASSERT_EQ(hero_inventory.add_item(oak_plank), FT_ERR_SUCCESS);
    FT_ASSERT_EQ(hero_inventory.add_item(iron_ingot), FT_ERR_SUCCESS);
    ft_crafting crafting_system;

    ft_crafting_ingredient oak_requirement;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, oak_requirement.initialize(101, 2, 1));
    ft_crafting_ingredient iron_requirement;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iron_requirement.initialize(102, 1, 2));
    ft_vector<ft_crafting_ingredient> sword_recipe;
    sword_recipe.push_back(oak_requirement);
    sword_recipe.push_back(iron_requirement);

    int const crafted_sword_recipe_id = 501;
    FT_ASSERT_EQ(crafting_system.register_recipe(crafted_sword_recipe_id, ft_move(sword_recipe)), FT_ERR_SUCCESS);

    ft_sharedptr<ft_item> crafted_sword(new (std::nothrow) ft_item());
    FT_ASSERT(crafted_sword.get() != ft_nullptr);
    crafted_sword->set_item_id(501);
    crafted_sword->set_max_stack(1);
    crafted_sword->set_stack_size(1);
    crafted_sword->set_rarity(3);
    crafted_sword->set_modifier1_id(1);
    crafted_sword->set_modifier1_value(5);

    ft_quest crafting_quest;
    crafting_quest.set_id(77);
    crafting_quest.set_phases(2);
    crafting_quest.set_current_phase(0);
    crafting_quest.set_reward_experience(120);
    hero_character.get_quests().insert(crafting_quest.get_id(), crafting_quest);

    int const quest_identifier = crafting_quest.get_id();
    int crafting_result_code = FT_ERR_GAME_GENERAL_ERROR;
    int combat_result_code = FT_ERR_GAME_GENERAL_ERROR;
    int completion_result_code = FT_ERR_GAME_GENERAL_ERROR;

    ft_sharedptr<ft_event> crafting_event(new (std::nothrow) ft_event());
    FT_ASSERT(crafting_event.get() != ft_nullptr);
    crafting_event->set_id(1);
    crafting_event->set_duration(1);
    crafting_event->set_modifier1(crafted_sword_recipe_id);
    crafting_event->set_callback(ft_function<void(ft_world&, ft_event&)>([&](ft_world &world_reference, ft_event &event_reference)
    {
        (void)world_reference;
        crafting_result_code = crafting_system.craft_item(hero_inventory, event_reference.get_modifier1(), crafted_sword);
        if (crafting_result_code == FT_ERR_SUCCESS)
        {
            Pair<int, ft_quest> *quest_entry;

            quest_entry = hero_character.get_quests().find(quest_identifier);
            if (quest_entry != hero_character.get_quests().end())
            {
                quest_entry->value.advance_phase();
            }
        }
    }));

    ft_sharedptr<ft_event> combat_event(new (std::nothrow) ft_event());
    FT_ASSERT(combat_event.get() != ft_nullptr);
    combat_event->set_id(2);
    combat_event->set_duration(3);
    combat_event->set_modifier1(18);
    combat_event->set_callback(ft_function<void(ft_world&, ft_event&)>([&](ft_world &world_reference, ft_event &event_reference)
    {
        (void)world_reference;
        enemy_character.take_damage(event_reference.get_modifier1(), FT_DAMAGE_PHYSICAL);
        combat_result_code = enemy_character.get_error();
    }));

    ft_sharedptr<ft_event> completion_event(new (std::nothrow) ft_event());
    FT_ASSERT(completion_event.get() != ft_nullptr);
    completion_event->set_id(3);
    completion_event->set_duration(4);
    completion_event->set_callback(ft_function<void(ft_world&, ft_event&)>([&](ft_world &world_reference, ft_event &event_reference)
    {
        (void)world_reference;
        (void)event_reference;
        Pair<int, ft_quest> *quest_entry;

        quest_entry = hero_character.get_quests().find(quest_identifier);
        if (quest_entry != hero_character.get_quests().end())
        {
            bool was_complete;

            was_complete = quest_entry->value.is_complete();
            if (!was_complete)
            {
                quest_entry->value.advance_phase();
            }
            if (quest_entry->value.is_complete())
            {
                hero_character.add_experience(quest_entry->value.get_reward_experience());
                completion_result_code = hero_character.get_error();
            }
        }
    }));
    FT_ASSERT_EQ(completion_event->get_error(), FT_ERR_SUCCESS);

    world_instance->schedule_event(crafting_event);
    world_instance->schedule_event(crafting_event);
    world_instance->schedule_event(combat_event);
    world_instance->schedule_event(completion_event);

    world_instance->update_events(world_instance, 1);
    FT_ASSERT_EQ(world_instance->get_event_scheduler()->size(), 3);
    FT_ASSERT_EQ(crafting_result_code, FT_ERR_SUCCESS);
    FT_ASSERT_EQ(hero_inventory.count_item(101), 1);
    FT_ASSERT_EQ(hero_inventory.count_item(102), 0);
    FT_ASSERT_EQ(hero_inventory.count_item(501), 1);
    Pair<int, ft_quest> *quest_entry_after_crafting = hero_character.get_quests().find(quest_identifier);
    FT_ASSERT(quest_entry_after_crafting != ft_nullptr);
    FT_ASSERT_EQ(quest_entry_after_crafting->value.get_current_phase(), 1);

    world_instance->update_events(world_instance, 1);
    FT_ASSERT_EQ(hero_inventory.count_item(501), 1);

    world_instance->update_events(world_instance, 1);
    FT_ASSERT_EQ(combat_result_code, FT_ERR_SUCCESS);
    FT_ASSERT_EQ(enemy_character.get_hit_points(), 22);

    world_instance->update_events(world_instance, 1);
    FT_ASSERT_EQ(completion_result_code, FT_ERR_SUCCESS);
    Pair<int, ft_quest> *quest_entry_after_completion = hero_character.get_quests().find(quest_identifier);
    FT_ASSERT(quest_entry_after_completion != ft_nullptr);
    FT_ASSERT(quest_entry_after_completion->value.is_complete());
    FT_ASSERT_EQ(hero_character.get_experience(), 120);
    FT_ASSERT_EQ(world_instance->get_event_scheduler()->size(), 0);
    FT_ASSERT_EQ(world_instance->get_error(), FT_ERR_SUCCESS);

    return (1);
}

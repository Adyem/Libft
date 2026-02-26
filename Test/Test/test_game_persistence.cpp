#include "../test_internal.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_item.hpp"
#include "../../Storage/kv_store.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static void cleanup_game_persistence_store(void)
{
    int error_code;

    error_code = FT_ERR_SUCCESS;
    cmp_file_delete("./Test/world_persistence_store.json", &error_code);
    return ;
}

FT_TEST(test_game_world_persistence_round_trip,
    "Game world save/load integrates kv_store persistence backends")
{
    const char *store_path = "./Test/world_persistence_store.json";
    int error_code = FT_ERR_SUCCESS;

    cleanup_game_persistence_store();
    su_file *store_file = su_fopen(store_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FT_ASSERT(store_file != ft_nullptr);
    const char *initial_content = "{\n  \"kv_store\": {\n  }\n}\n";
    size_t initial_length = ft_strlen(initial_content);
    FT_ASSERT_EQ(su_fwrite(initial_content, 1, initial_length, store_file), initial_length);
    FT_ASSERT_EQ(su_fclose(store_file), 0);

    kv_store persistence_store(store_path);

    ft_world world_instance;
    FT_ASSERT_EQ(world_instance.get_error(), FT_ERR_SUCCESS);

    ft_sharedptr<ft_event> scheduled_event(new (std::nothrow) ft_event());
    FT_ASSERT(scheduled_event.get() != ft_nullptr);
    FT_ASSERT_EQ(scheduled_event->get_error(), FT_ERR_SUCCESS);
    scheduled_event->set_id(7);
    scheduled_event->set_duration(5);
    FT_ASSERT_EQ(scheduled_event->get_error(), FT_ERR_SUCCESS);
    world_instance.schedule_event(scheduled_event);
    FT_ASSERT_EQ(world_instance.get_error(), FT_ERR_SUCCESS);

    ft_character hero;
    hero.set_hit_points(64);
    hero.set_damage_rule(FT_DAMAGE_RULE_FLAT);
    FT_ASSERT_EQ(hero.get_error(), FT_ERR_SUCCESS);

    ft_inventory backpack;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, backpack.initialize(6, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, backpack.get_error());
    ft_sharedptr<ft_item> potion(new (std::nothrow) ft_item());
    FT_ASSERT(potion.get() != ft_nullptr);
    FT_ASSERT_EQ(potion->get_error(), FT_ERR_SUCCESS);
    potion->set_item_id(200);
    potion->set_max_stack(5);
    potion->set_stack_size(2);
    potion->set_modifier1_id(3);
    potion->set_modifier1_value(25);
    FT_ASSERT_EQ(potion->get_error(), FT_ERR_SUCCESS);
    FT_ASSERT_EQ(backpack.add_item(potion), FT_ERR_SUCCESS);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, backpack.get_error());

    FT_ASSERT_EQ(world_instance.save_to_store(persistence_store, "slot-primary", hero, backpack), FT_ERR_SUCCESS);
    FT_ASSERT_EQ(world_instance.get_error(), FT_ERR_SUCCESS);

    ft_character restored_hero;
    ft_inventory restored_inventory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, restored_inventory.initialize(1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, restored_inventory.get_error());

    FT_ASSERT_EQ(world_instance.load_from_store(persistence_store, "slot-primary", restored_hero, restored_inventory), FT_ERR_SUCCESS);
    FT_ASSERT_EQ(world_instance.get_error(), FT_ERR_SUCCESS);

    FT_ASSERT_EQ(restored_hero.get_hit_points(), hero.get_hit_points());
    FT_ASSERT_EQ(restored_inventory.get_used(), backpack.get_used());
    FT_ASSERT_EQ(restored_inventory.count_item(200), backpack.count_item(200));

    ft_vector<ft_sharedptr<ft_event> > restored_events;
    ft_sharedptr<ft_event_scheduler> &restored_scheduler = world_instance.get_event_scheduler();
    FT_ASSERT(restored_scheduler.get() != ft_nullptr);
    restored_scheduler->dump_events(restored_events);
    FT_ASSERT_EQ(restored_events.size(), static_cast<size_t>(1));
    FT_ASSERT(restored_events[0].get() != ft_nullptr);
    FT_ASSERT_EQ(restored_events[0]->get_id(), scheduled_event->get_id());

    FT_ASSERT_EQ(cmp_file_delete(store_path, &error_code), 0);
    cleanup_game_persistence_store();

    return (1);
}

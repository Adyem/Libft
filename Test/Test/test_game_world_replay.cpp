#include "../../Game/game_world_replay.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_character.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_event.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <new>

FT_TEST(test_game_world_replay_capture_restore_replay,
    "ft_world_replay_session captures snapshots, restores, and replays deterministically")
{
    ft_sharedptr<ft_world> world_pointer(new (std::nothrow) ft_world());
    ft_world_replay_session session;
    ft_character hero_character;
    ft_inventory hero_inventory(4);
    ft_sharedptr<ft_item> healing_potion(new (std::nothrow) ft_item());
    ft_sharedptr<ft_event> damage_event(new (std::nothrow) ft_event());
    int damage_invocations;
    int capture_result;
    int restore_result;
    int replay_result;

    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(world_pointer.get_error(), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(session.get_error(), FT_ER_SUCCESSS);

    hero_character.set_hit_points(20);
    FT_ASSERT_EQ(hero_character.get_error(), FT_ER_SUCCESSS);
    hero_character.set_damage_rule(FT_DAMAGE_RULE_FLAT);
    FT_ASSERT_EQ(hero_character.get_error(), FT_ER_SUCCESSS);

    FT_ASSERT_EQ(hero_inventory.get_error(), FT_ER_SUCCESSS);
    healing_potion->set_item_id(301);
    healing_potion->set_max_stack(5);
    healing_potion->set_stack_size(1);
    FT_ASSERT_EQ(healing_potion->get_error(), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(hero_inventory.add_item(healing_potion), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(hero_inventory.get_error(), FT_ER_SUCCESSS);

    damage_invocations = 0;
    damage_event->set_id(55);
    damage_event->set_duration(1);
    damage_event->set_callback(ft_function<void(ft_world&, ft_event&)>([&hero_character, &damage_invocations](ft_world &world_ref, ft_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        hero_character.take_damage(5, FT_DAMAGE_PHYSICAL);
        damage_invocations = damage_invocations + 1;
        return ;
    }));
    FT_ASSERT_EQ(damage_event->get_error(), FT_ER_SUCCESSS);

    world_pointer->schedule_event(damage_event);
    FT_ASSERT_EQ(world_pointer->get_error(), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(world_pointer->get_event_scheduler()->size(), static_cast<size_t>(1));

    capture_result = session.capture_snapshot(*world_pointer, hero_character, hero_inventory);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, capture_result);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, session.get_error());

    world_pointer->update_events(world_pointer, 1);
    FT_ASSERT_EQ(world_pointer->get_error(), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(hero_character.get_hit_points(), 15);
    FT_ASSERT_EQ(damage_invocations, 1);
    FT_ASSERT_EQ(world_pointer->get_event_scheduler()->size(), static_cast<size_t>(0));

    restore_result = session.restore_snapshot(world_pointer, hero_character, hero_inventory);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, restore_result);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, session.get_error());
    FT_ASSERT_EQ(hero_character.get_hit_points(), 20);
    FT_ASSERT_EQ(hero_inventory.count_item(301), 1);
    FT_ASSERT_EQ(world_pointer->get_event_scheduler()->size(), static_cast<size_t>(1));

    hero_character.set_hit_points(25);
    FT_ASSERT_EQ(hero_character.get_error(), FT_ER_SUCCESSS);
    replay_result = session.replay_ticks(world_pointer, hero_character, hero_inventory, 1, ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, replay_result);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, session.get_error());
    FT_ASSERT_EQ(hero_character.get_hit_points(), 15);
    FT_ASSERT_EQ(hero_inventory.count_item(301), 1);
    FT_ASSERT_EQ(world_pointer->get_event_scheduler()->size(), static_cast<size_t>(0));
    FT_ASSERT_EQ(damage_invocations, 2);

    return (1);
}

FT_TEST(test_game_world_replay_import_export_clear,
    "ft_world_replay_session imports, exports, and clears snapshots while propagating errors")
{
    ft_sharedptr<ft_world> world_pointer(new (std::nothrow) ft_world());
    ft_world_replay_session session;
    ft_world_replay_session imported_session;
    ft_character hero_character;
    ft_inventory hero_inventory(2);
    ft_string exported_snapshot;
    ft_string cleared_snapshot;
    int restore_result;

    FT_ASSERT(world_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(world_pointer.get_error(), FT_ER_SUCCESSS);

    restore_result = session.restore_snapshot(world_pointer, hero_character, hero_inventory);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, restore_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, session.get_error());

    hero_character.set_hit_points(33);
    FT_ASSERT_EQ(hero_character.get_error(), FT_ER_SUCCESSS);

    FT_ASSERT_EQ(session.capture_snapshot(*world_pointer, hero_character, hero_inventory), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(session.get_error(), FT_ER_SUCCESSS);

    FT_ASSERT_EQ(session.export_snapshot(exported_snapshot), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(session.get_error(), FT_ER_SUCCESSS);
    FT_ASSERT(exported_snapshot.empty() == false);

    session.clear_snapshot();
    FT_ASSERT_EQ(session.get_error(), FT_ER_SUCCESSS);

    FT_ASSERT_EQ(session.export_snapshot(cleared_snapshot), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(session.get_error(), FT_ER_SUCCESSS);
    FT_ASSERT(cleared_snapshot.empty());

    FT_ASSERT_EQ(imported_session.import_snapshot(exported_snapshot), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(imported_session.get_error(), FT_ER_SUCCESSS);

    hero_character.set_hit_points(10);
    FT_ASSERT_EQ(hero_character.get_error(), FT_ER_SUCCESSS);

    FT_ASSERT_EQ(imported_session.restore_snapshot(world_pointer, hero_character, hero_inventory), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(imported_session.get_error(), FT_ER_SUCCESSS);
    FT_ASSERT_EQ(hero_character.get_hit_points(), 33);

    return (1);
}

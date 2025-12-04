#include "../../Game/game_world_registry.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_world_registry_register_world_restores_errno, "register_world keeps incoming errno unchanged on success")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    ft_world_region world;

    region_ids.push_back(5);
    world = ft_world_region(4, region_ids);
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    FT_ASSERT_EQ(ER_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, registry.get_error());
    return (1);
}

FT_TEST(test_world_registry_copy_constructor_copies_entries_and_error, "copy constructor duplicates stored maps and error code")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    ft_region_definition region(9, ft_string("gorge"), ft_string("deep canyon"), 2);
    ft_world_region world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;
    ft_world_registry copied;

    region_ids.push_back(9);
    world = ft_world_region(3, region_ids);
    registry.register_region(region);
    registry.register_world(world);
    registry.fetch_world(33, fetched_world);
    copied = ft_world_registry(registry);
    FT_ASSERT_EQ(ER_SUCCESS, copied.fetch_region(9, fetched_region));
    FT_ASSERT_EQ(ft_string("gorge"), fetched_region.get_name());
    FT_ASSERT_EQ(ER_SUCCESS, copied.fetch_world(3, fetched_world));
    FT_ASSERT_EQ(9, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, copied.get_error());
    return (1);
}

FT_TEST(test_world_registry_move_constructor_transfers_entries_and_clears_source, "move constructor moves registry data and reset source")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    ft_region_definition region(4, ft_string("tower"), ft_string("stone spire"), 1);
    ft_world_region world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    region_ids.push_back(4);
    world = ft_world_region(2, region_ids);
    registry.register_region(region);
    registry.register_world(world);
    registry.fetch_region(90, fetched_region);

    ft_world_registry moved(ft_move(registry));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_region(4, fetched_region));
    FT_ASSERT_EQ(ft_string("tower"), fetched_region.get_name());
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_world(2, fetched_world));
    FT_ASSERT_EQ(4, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(ER_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(4, fetched_region));
    return (1);
}

FT_TEST(test_world_registry_copy_assignment_overwrites_entries_and_error, "copy assignment replaces data and propagates error state")
{
    ft_world_registry source;
    ft_world_registry destination;
    ft_vector<int> source_region_ids;
    ft_vector<int> destination_region_ids;
    ft_region_definition source_region(12, ft_string("harbor"), ft_string("port city"), 7);
    ft_region_definition destination_region(1, ft_string("plains"), ft_string("flat land"), 1);
    ft_world_region source_world;
    ft_world_region destination_world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    source_region_ids.push_back(12);
    destination_region_ids.push_back(1);
    source_world = ft_world_region(8, source_region_ids);
    destination_world = ft_world_region(2, destination_region_ids);
    source.register_region(source_region);
    source.register_world(source_world);
    source.fetch_world(99, fetched_world);
    destination.register_region(destination_region);
    destination.register_world(destination_world);

    destination = source;
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_region(12, fetched_region));
    FT_ASSERT_EQ(ft_string("harbor"), fetched_region.get_name());
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_world(8, fetched_world));
    FT_ASSERT_EQ(12, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, destination.get_error());
    return (1);
}

FT_TEST(test_world_registry_move_assignment_transfers_entries_and_resets_source_error, "move assignment transfers storage and resets origin state")
{
    ft_world_registry source;
    ft_world_registry destination;
    ft_vector<int> region_ids;
    ft_region_definition region(16, ft_string("desert"), ft_string("hot sands"), 5);
    ft_world_region world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    region_ids.push_back(16);
    world = ft_world_region(6, region_ids);
    source.register_region(region);
    source.register_world(world);
    destination.fetch_region(3, fetched_region);

    destination = ft_move(source);
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_region(16, fetched_region));
    FT_ASSERT_EQ(ft_string("desert"), fetched_region.get_name());
    FT_ASSERT_EQ(ER_SUCCESS, destination.fetch_world(6, fetched_world));
    FT_ASSERT_EQ(16, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.fetch_world(6, fetched_world));
    return (1);
}

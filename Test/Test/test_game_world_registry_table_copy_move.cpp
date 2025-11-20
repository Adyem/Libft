#include "../../Game/game_world_registry.hpp"
#include "../../System_utils/test_runner.hpp"

static int register_entries(ft_world_registry &registry)
{
    ft_region_definition region;
    ft_world_region world;
    ft_vector<int> region_ids;

    region = ft_region_definition(4, ft_string("forest"), ft_string("trees"), 2);
    FT_ASSERT_EQ(ER_SUCCESS, registry.register_region(region));
    region_ids.push_back(4);
    world = ft_world_region(1, region_ids);
    FT_ASSERT_EQ(ER_SUCCESS, registry.register_world(world));
    return (1);
}

FT_TEST(test_world_registry_copy_semantics, "world registry copy constructor and assignment")
{
    ft_world_registry registry;
    ft_world_registry copied;
    ft_world_registry assigned;
    ft_region_definition region;

    register_entries(registry);
    copied = ft_world_registry(registry);
    FT_ASSERT_EQ(ER_SUCCESS, copied.fetch_region(4, region));
    FT_ASSERT_EQ(ft_string("forest"), region.get_name());

    assigned = ft_world_registry();
    assigned = registry;
    FT_ASSERT_EQ(ER_SUCCESS, assigned.fetch_region(4, region));
    FT_ASSERT_EQ(ft_string("trees"), region.get_description());
    FT_ASSERT(assigned.get_world_regions().find(99) == assigned.get_world_regions().end());
    return (1);
}

FT_TEST(test_world_registry_move_semantics, "world registry move constructor and assignment")
{
    ft_world_registry registry;
    ft_world_registry moved;
    ft_world_registry moved_assigned;
    ft_world_region world;

    register_entries(registry);
    moved = ft_world_registry(ft_move(registry));
    FT_ASSERT_EQ(ER_SUCCESS, moved.fetch_world(1, world));
    FT_ASSERT(registry.get_regions().empty());
    FT_ASSERT(registry.get_world_regions().empty());
    FT_ASSERT_EQ(ER_SUCCESS, registry.get_error());

    moved_assigned = ft_world_registry();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(ER_SUCCESS, moved_assigned.fetch_world(1, world));
    FT_ASSERT(moved.get_regions().empty());
    FT_ASSERT(moved.get_world_regions().empty());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}

#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_region_definition_copy_move, "copy and move region definitions")
{
    ft_region_definition region;
    ft_region_definition copied;
    ft_region_definition assigned;
    ft_region_definition moved;
    ft_region_definition moved_assigned;

    region = ft_region_definition(4, ft_string("forest"), ft_string("dense trees"), 6);

    copied = ft_region_definition(region);
    FT_ASSERT_EQ(4, copied.get_region_id());
    FT_ASSERT_EQ(ft_string("forest"), copied.get_name());
    FT_ASSERT_EQ(ft_string("dense trees"), copied.get_description());

    assigned = ft_region_definition();
    assigned = region;
    FT_ASSERT_EQ(6, assigned.get_recommended_level());

    moved = ft_region_definition(ft_move(region));
    FT_ASSERT_EQ(ft_string("forest"), moved.get_name());
    FT_ASSERT(region.get_name().empty());
    FT_ASSERT_EQ(0, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, region.get_error());

    moved_assigned = ft_region_definition();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(ft_string("dense trees"), moved_assigned.get_description());
    FT_ASSERT(moved.get_name().empty());
    FT_ASSERT_EQ(0, moved.get_region_id());
    return (1);
}

FT_TEST(test_world_region_copy_move, "copy and move world regions")
{
    ft_vector<int> region_ids;
    ft_world_region world;
    ft_world_region copied;
    ft_world_region assigned;
    ft_world_region moved;
    ft_world_region moved_assigned;

    region_ids.push_back(3);
    region_ids.push_back(4);
    world = ft_world_region(9, region_ids);

    copied = ft_world_region(world);
    FT_ASSERT_EQ(9, copied.get_world_id());
    FT_ASSERT_EQ(2, copied.get_region_ids().size());

    assigned = ft_world_region();
    assigned = world;
    FT_ASSERT_EQ(4, assigned.get_region_ids()[1]);

    moved = ft_world_region(ft_move(world));
    FT_ASSERT_EQ(2, moved.get_region_ids().size());
    FT_ASSERT(world.get_region_ids().empty());
    FT_ASSERT_EQ(0, world.get_world_id());

    moved_assigned = ft_world_region();
    moved_assigned = ft_move(moved);
    FT_ASSERT_EQ(3, moved_assigned.get_region_ids()[0]);
    FT_ASSERT(moved.get_region_ids().empty());
    FT_ASSERT_EQ(0, moved.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}

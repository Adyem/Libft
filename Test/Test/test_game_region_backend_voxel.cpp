#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#define GAME_USE_VOXEL_REGION_BACKEND 1

#include "../test_internal.hpp"
#include "../../Modules/Game/game_region_backend.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_region_backend_macro_selects_voxel_backend)
{
    FT_ASSERT_EQ(1, GAME_ACTIVE_REGION_BACKEND_VOXEL);
    FT_ASSERT_EQ(sizeof(game_voxel_region), sizeof(game_active_region_backend));
    return (1);
}

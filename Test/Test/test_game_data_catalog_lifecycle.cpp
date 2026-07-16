#include "../test_internal.hpp"

#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_data_catalog_thread_safe_lifecycle)
{
    game_data_catalog catalog;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), catalog.get_item_definitions().size());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), catalog.get_recipes().size());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), catalog.get_loadouts().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.destroy());
    return (1);
}

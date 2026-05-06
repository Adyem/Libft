#include "../../Modules/Game/game_crafting.hpp"
#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_crafting_placeholder)
{
    return (1);
}

FT_TEST(test_crafting_thread_safety_toggle)
{
    game_crafting crafting_instance;
    game_crafting_ingredient ingredient_instance;
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, crafting_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient_instance.initialize(1, 1, 1));

    FT_ASSERT_EQ(false, crafting_instance.is_thread_safe());
    FT_ASSERT_EQ(false, ingredient_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, crafting_instance.enable_thread_safety());
    FT_ASSERT_EQ(true, crafting_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, crafting_instance.disable_thread_safety());
    FT_ASSERT_EQ(false, crafting_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient_instance.enable_thread_safety());
    FT_ASSERT_EQ(true, ingredient_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient_instance.disable_thread_safety());
    FT_ASSERT_EQ(false, ingredient_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, crafting_instance.destroy());
    return (1);
}

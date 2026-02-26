#include "../../Game/game_crafting.hpp"
#include "../test_internal.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_crafting_placeholder, "crafting placeholder")
{
    return (1);
}

FT_TEST(test_crafting_thread_safety_toggle, "crafting thread safety toggle")
{
    ft_crafting crafting_instance;
    ft_crafting_ingredient ingredient_instance;
    cma_set_alloc_limit(0);

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
    return (1);
}

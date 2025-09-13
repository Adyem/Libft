#include "../../Game/crafting.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_crafting_success, "crafting succeeds")
{
    ft_crafting crafting;
    ft_vector<int> ingredients;
    ingredients.push_back(1);
    ingredients.push_back(2);
    crafting.register_recipe(1, ft_move(ingredients));

    ft_inventory inventory(5);
    ft_item ingredient_one;
    ingredient_one.set_item_id(1);
    ingredient_one.set_max_stack(1);
    ingredient_one.set_current_stack(1);
    ft_item ingredient_two;
    ingredient_two.set_item_id(2);
    ingredient_two.set_max_stack(1);
    ingredient_two.set_current_stack(1);
    inventory.add_item(ingredient_one);
    inventory.add_item(ingredient_two);

    ft_item crafted;
    crafted.set_item_id(3);
    crafted.set_max_stack(1);
    crafted.set_current_stack(1);

    FT_ASSERT_EQ(ER_SUCCESS, crafting.craft_item(inventory, 1, crafted));
    FT_ASSERT_EQ(1, inventory.count_item(3));
    FT_ASSERT_EQ(0, inventory.count_item(1));
    FT_ASSERT_EQ(0, inventory.count_item(2));
    return (1);
}

FT_TEST(test_crafting_missing_ingredient, "crafting fails with missing ingredient")
{
    ft_crafting crafting;
    ft_vector<int> ingredients;
    ingredients.push_back(1);
    ingredients.push_back(2);
    crafting.register_recipe(1, ft_move(ingredients));

    ft_inventory inventory(5);
    ft_item ingredient_one;
    ingredient_one.set_item_id(1);
    ingredient_one.set_max_stack(1);
    ingredient_one.set_current_stack(1);
    inventory.add_item(ingredient_one);

    ft_item crafted;
    crafted.set_item_id(3);
    crafted.set_max_stack(1);
    crafted.set_current_stack(1);

    FT_ASSERT_EQ(GAME_GENERAL_ERROR, crafting.craft_item(inventory, 1, crafted));
    FT_ASSERT_EQ(0, inventory.count_item(3));
    FT_ASSERT_EQ(1, inventory.count_item(1));
    return (1);
}

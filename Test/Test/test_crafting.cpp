#include "../../Game/game_crafting.hpp"
#include "../../System_utils/test_runner.hpp"
#include <utility>

FT_TEST(test_crafting_success, "crafting succeeds")
{
    ft_crafting crafting;
    ft_vector<ft_crafting_ingredient> ingredients;
    ft_crafting_ingredient ingredient_a = {1, 1, -1};
    ft_crafting_ingredient ingredient_b = {2, 1, -1};
    ingredients.push_back(ingredient_a);
    ingredients.push_back(ingredient_b);
    crafting.register_recipe(1, std::move(ingredients));

    ft_inventory inventory(5);
    ft_item ingredient_one;
    ingredient_one.set_item_id(1);
    ingredient_one.set_max_stack(1);
    ingredient_one.set_stack_size(1);
    ingredient_one.set_rarity(0);
    ft_item ingredient_two;
    ingredient_two.set_item_id(2);
    ingredient_two.set_max_stack(1);
    ingredient_two.set_stack_size(1);
    ingredient_two.set_rarity(0);
    inventory.add_item(ingredient_one);
    inventory.add_item(ingredient_two);

    ft_item crafted;
    crafted.set_item_id(3);
    crafted.set_max_stack(1);
    crafted.set_stack_size(1);
    crafted.set_rarity(0);

    FT_ASSERT_EQ(ER_SUCCESS, crafting.craft_item(inventory, 1, crafted));
    FT_ASSERT_EQ(1, inventory.count_item(3));
    FT_ASSERT_EQ(0, inventory.count_item(1));
    FT_ASSERT_EQ(0, inventory.count_item(2));
    return (1);
}

FT_TEST(test_crafting_missing_ingredient, "crafting fails with missing ingredient")
{
    ft_crafting crafting;
    ft_vector<ft_crafting_ingredient> ingredients;
    ft_crafting_ingredient ingredient_a = {1, 1, -1};
    ft_crafting_ingredient ingredient_b = {2, 1, -1};
    ingredients.push_back(ingredient_a);
    ingredients.push_back(ingredient_b);
    crafting.register_recipe(1, std::move(ingredients));

    ft_inventory inventory(5);
    ft_item ingredient_one;
    ingredient_one.set_item_id(1);
    ingredient_one.set_max_stack(1);
    ingredient_one.set_stack_size(1);
    ingredient_one.set_rarity(0);
    inventory.add_item(ingredient_one);

    ft_item crafted;
    crafted.set_item_id(3);
    crafted.set_max_stack(1);
    crafted.set_stack_size(1);
    crafted.set_rarity(0);

    FT_ASSERT_EQ(GAME_GENERAL_ERROR, crafting.craft_item(inventory, 1, crafted));
    FT_ASSERT_EQ(0, inventory.count_item(3));
    FT_ASSERT_EQ(1, inventory.count_item(1));
    return (1);
}

FT_TEST(test_inventory_rarity, "inventory queries rarity")
{
    ft_inventory inventory(5);
    ft_item item_one;
    item_one.set_item_id(1);
    item_one.set_max_stack(1);
    item_one.set_stack_size(1);
    item_one.set_rarity(0);
    ft_item item_two;
    item_two.set_item_id(2);
    item_two.set_max_stack(1);
    item_two.set_stack_size(1);
    item_two.set_rarity(1);
    inventory.add_item(item_one);
    inventory.add_item(item_two);
    FT_ASSERT_EQ(1, inventory.count_rarity(0));
    FT_ASSERT(inventory.has_rarity(1));
    return (1);
}

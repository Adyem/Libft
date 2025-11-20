#include "../../Game/game_data_catalog.hpp"
#include "../../System_utils/test_runner.hpp"

static void populate_ingredient(ft_crafting_ingredient &ingredient, int item_id, int count, int rarity)
{
    ingredient.set_item_id(item_id);
    ingredient.set_count(count);
    ingredient.set_rarity(rarity);
    return ;
}

FT_TEST(test_item_definition_setters, "update item definition fields")
{
    ft_item_definition definition;

    definition.set_item_id(4);
    definition.set_rarity(2);
    definition.set_max_stack(15);
    definition.set_width(1);
    definition.set_height(2);
    definition.set_weight(9);
    definition.set_slot_requirement(5);

    FT_ASSERT_EQ(4, definition.get_item_id());
    FT_ASSERT_EQ(2, definition.get_rarity());
    FT_ASSERT_EQ(15, definition.get_max_stack());
    FT_ASSERT_EQ(1, definition.get_width());
    FT_ASSERT_EQ(2, definition.get_height());
    FT_ASSERT_EQ(9, definition.get_weight());
    FT_ASSERT_EQ(5, definition.get_slot_requirement());
    FT_ASSERT_EQ(ER_SUCCESS, definition.get_error());
    return (1);
}

FT_TEST(test_recipe_blueprint_setters, "update recipe fields and ingredients")
{
    ft_vector<ft_crafting_ingredient> ingredients;
    ft_crafting_ingredient ingredient;
    ft_recipe_blueprint recipe;

    populate_ingredient(ingredient, 10, 3, 1);
    ingredients.push_back(ingredient);
    populate_ingredient(ingredient, 11, 4, 2);
    ingredients.push_back(ingredient);

    recipe.set_recipe_id(6);
    recipe.set_result_item_id(30);
    recipe.set_ingredients(ingredients);

    FT_ASSERT_EQ(6, recipe.get_recipe_id());
    FT_ASSERT_EQ(30, recipe.get_result_item_id());
    FT_ASSERT_EQ(2u, recipe.get_ingredients().size());
    FT_ASSERT_EQ(10, recipe.get_ingredients()[0].get_item_id());
    FT_ASSERT_EQ(11, recipe.get_ingredients()[1].get_item_id());
    FT_ASSERT_EQ(ER_SUCCESS, recipe.get_error());
    return (1);
}

FT_TEST(test_loadout_entry_setters, "update loadout entry fields")
{
    ft_loadout_entry entry;

    entry.set_slot(3);
    entry.set_item_id(14);
    entry.set_quantity(5);

    FT_ASSERT_EQ(3, entry.get_slot());
    FT_ASSERT_EQ(14, entry.get_item_id());
    FT_ASSERT_EQ(5, entry.get_quantity());
    FT_ASSERT_EQ(ER_SUCCESS, entry.get_error());
    return (1);
}

FT_TEST(test_loadout_blueprint_setters, "update loadout blueprint fields and entries")
{
    ft_vector<ft_loadout_entry> entries;
    ft_loadout_entry entry;
    ft_loadout_blueprint blueprint;

    entry.set_slot(0);
    entry.set_item_id(1);
    entry.set_quantity(2);
    entries.push_back(entry);
    entry.set_slot(2);
    entry.set_item_id(3);
    entry.set_quantity(4);
    entries.push_back(entry);

    blueprint.set_loadout_id(18);
    blueprint.set_entries(entries);

    FT_ASSERT_EQ(18, blueprint.get_loadout_id());
    FT_ASSERT_EQ(2u, blueprint.get_entries().size());
    FT_ASSERT_EQ(0, blueprint.get_entries()[0].get_slot());
    FT_ASSERT_EQ(3, blueprint.get_entries()[1].get_item_id());
    FT_ASSERT_EQ(ER_SUCCESS, blueprint.get_error());
    return (1);
}

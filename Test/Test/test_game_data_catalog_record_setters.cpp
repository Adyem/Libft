#include "../test_internal.hpp"
#include "../../Game/game_data_catalog.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int populate_ingredient(game_crafting_ingredient &ingredient, int item_id, int count, int rarity)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient.initialize(item_id, count, rarity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredient.get_error());
    ingredient.set_item_id(item_id);
    ingredient.set_count(count);
    ingredient.set_rarity(rarity);
    return (1);
}

FT_TEST(test_item_definition_setters)
{
    game_item_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());

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
    return (1);
}

FT_TEST(test_recipe_blueprint_setters)
{
    ft_vector<game_crafting_ingredient> ingredients;
    game_crafting_ingredient ingredient;
    game_recipe_blueprint recipe;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.get_error());
    FT_ASSERT_EQ(1, populate_ingredient(ingredient, 10, 3, 1));
    ingredients.push_back(ingredient);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.get_error());
    FT_ASSERT_EQ(1, populate_ingredient(ingredient, 11, 4, 2));
    ingredients.push_back(ingredient);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, recipe.initialize());
    recipe.set_recipe_id(6);
    recipe.set_result_item_id(30);
    recipe.set_ingredients(ingredients);

    FT_ASSERT_EQ(6, recipe.get_recipe_id());
    FT_ASSERT_EQ(30, recipe.get_result_item_id());
    FT_ASSERT_EQ(2u, recipe.get_ingredients().size());
    FT_ASSERT_EQ(10, recipe.get_ingredients()[0].get_item_id());
    FT_ASSERT_EQ(11, recipe.get_ingredients()[1].get_item_id());
    return (1);
}

FT_TEST(test_loadout_entry_setters)
{
    game_loadout_entry entry;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.initialize());

    entry.set_slot(3);
    entry.set_item_id(14);
    entry.set_quantity(5);

    FT_ASSERT_EQ(3, entry.get_slot());
    FT_ASSERT_EQ(14, entry.get_item_id());
    FT_ASSERT_EQ(5, entry.get_quantity());
    return (1);
}

FT_TEST(test_loadout_blueprint_setters)
{
    ft_vector<game_loadout_entry> entries;
    game_loadout_entry first_entry;
    game_loadout_entry second_entry;
    game_loadout_blueprint blueprint;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, blueprint.initialize());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_entry.initialize(0, 1, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_entry.get_error());
    first_entry.set_slot(0);
    first_entry.set_item_id(1);
    first_entry.set_quantity(2);
    entries.push_back(first_entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_entry.initialize(2, 3, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_entry.get_error());
    second_entry.set_slot(2);
    second_entry.set_item_id(3);
    second_entry.set_quantity(4);
    entries.push_back(second_entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());

    blueprint.set_loadout_id(18);
    blueprint.set_entries(entries);

    FT_ASSERT_EQ(18, blueprint.get_loadout_id());
    FT_ASSERT_EQ(2u, blueprint.get_entries().size());
    FT_ASSERT_EQ(0, blueprint.get_entries()[0].get_slot());
    FT_ASSERT_EQ(3, blueprint.get_entries()[1].get_item_id());
    return (1);
}

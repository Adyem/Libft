#include "../test_internal.hpp"
#include "../../Game/game_data_catalog.hpp"
#include "../../Game/game_crafting.hpp"
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

static int populate_loadout_entry(game_loadout_entry &entry, int slot, int item_id, int quantity)
{
    if (entry._initialised_state == FT_CLASS_STATE_INITIALISED)
        FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.initialize(slot, item_id, quantity));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.get_error());
    return (1);
}

static int assert_item_definition_equals(const game_item_definition &definition, int item_id, int rarity,
        int max_stack, int width, int height, int weight, int slot_requirement)
{
    FT_ASSERT_EQ(item_id, definition.get_item_id());
    FT_ASSERT_EQ(rarity, definition.get_rarity());
    FT_ASSERT_EQ(max_stack, definition.get_max_stack());
    FT_ASSERT_EQ(width, definition.get_width());
    FT_ASSERT_EQ(height, definition.get_height());
    FT_ASSERT_EQ(weight, definition.get_weight());
    FT_ASSERT_EQ(slot_requirement, definition.get_slot_requirement());
    return (1);
}

static int assert_single_ingredient(const game_recipe_blueprint &recipe, int item_id, int count, int rarity)
{
    const ft_vector<game_crafting_ingredient> &ingredients = recipe.get_ingredients();
    FT_ASSERT_EQ(1u, ingredients.size());
    FT_ASSERT_EQ(item_id, ingredients[0].get_item_id());
    FT_ASSERT_EQ(count, ingredients[0].get_count());
    FT_ASSERT_EQ(rarity, ingredients[0].get_rarity());
    return (1);
}

static int assert_loadout_entry_values(const game_loadout_blueprint &loadout, size_t index,
        int slot, int item_id, int quantity)
{
    const ft_vector<game_loadout_entry> &entries = loadout.get_entries();
    FT_ASSERT_EQ(slot, entries[index].get_slot());
    FT_ASSERT_EQ(item_id, entries[index].get_item_id());
    FT_ASSERT_EQ(quantity, entries[index].get_quantity());
    return (1);
}

FT_TEST(test_catalog_register_and_fetch_item)
{
    game_data_catalog catalog;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    game_item_definition definition;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize(1, 2, 5, 3, 4, 6, 7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.get_error());
    game_item_definition fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.register_item_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.fetch_item_definition(1, fetched));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    assert_item_definition_equals(fetched, 1, 2, 5, 3, 4, 6, 7);
    return (1);
}

FT_TEST(test_catalog_fetch_missing_item)
{
    game_data_catalog catalog;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    game_item_definition definition;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, catalog.fetch_item_definition(99, definition));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, catalog.get_error());
    return (1);
}

FT_TEST(test_catalog_recipe_copy_isolated)
{
    game_data_catalog catalog;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    ft_vector<game_crafting_ingredient> ingredients;
    game_crafting_ingredient ingredient;
    game_recipe_blueprint recipe;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recipe.initialize(5, 42, ingredients));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recipe.get_error());
    game_recipe_blueprint first_fetch;
    game_recipe_blueprint second_fetch;

    FT_ASSERT_EQ(1, populate_ingredient(ingredient, 2, 3, 1));
    ingredients.push_back(ingredient);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ingredients.get_error());
    recipe.set_ingredients(ingredients);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.register_recipe(recipe));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.fetch_recipe(5, first_fetch));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    assert_single_ingredient(first_fetch, 2, 3, 1);

    ft_vector<game_crafting_ingredient> &first_ingredients = first_fetch.get_ingredients();
    first_ingredients[0].set_count(9);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.fetch_recipe(5, second_fetch));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    assert_single_ingredient(second_fetch, 2, 3, 1);
    return (1);
}

FT_TEST(test_catalog_loadout_copy_isolated)
{
    game_data_catalog catalog;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    ft_vector<game_loadout_entry> entries;
    game_loadout_blueprint loadout;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loadout.initialize(7, entries));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loadout.get_error());
    game_loadout_blueprint first_fetch;
    game_loadout_blueprint second_fetch;

    game_loadout_entry entry;
    FT_ASSERT_EQ(1, populate_loadout_entry(entry, 0, 10, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.get_error());
    entries.push_back(entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.destroy());
    FT_ASSERT_EQ(1, populate_loadout_entry(entry, 1, 11, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.get_error());
    entries.push_back(entry);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.get_error());
    loadout.set_entries(entries);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.register_loadout(loadout));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.fetch_loadout(7, first_fetch));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    const ft_vector<game_loadout_entry> &first_entries = first_fetch.get_entries();
    FT_ASSERT_EQ(2u, first_entries.size());
    assert_loadout_entry_values(first_fetch, 0, 0, 10, 2);
    assert_loadout_entry_values(first_fetch, 1, 1, 11, 1);

    ft_vector<game_loadout_entry> &first_mutable_entries = first_fetch.get_entries();
    first_mutable_entries[0].set_quantity(5);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.fetch_loadout(7, second_fetch));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, catalog.get_error());
    assert_loadout_entry_values(second_fetch, 0, 0, 10, 2);
    assert_loadout_entry_values(second_fetch, 1, 1, 11, 1);
    return (1);
}

#include "../../Game/game_data_catalog.hpp"
#include "../../Game/game_crafting.hpp"
#include "../../System_utils/test_runner.hpp"

static void populate_ingredient(ft_crafting_ingredient &ingredient, int item_id, int count, int rarity)
{
    ingredient.set_item_id(item_id);
    ingredient.set_count(count);
    ingredient.set_rarity(rarity);
    return ;
}

static void populate_loadout_entry(ft_loadout_entry &entry, int slot, int item_id, int quantity)
{
    entry.set_slot(slot);
    entry.set_item_id(item_id);
    entry.set_quantity(quantity);
    return ;
}

static int assert_item_definition_equals(const ft_item_definition &definition, int item_id, int rarity,
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

static int assert_single_ingredient(const ft_recipe_blueprint &recipe, int item_id, int count, int rarity)
{
    const ft_vector<ft_crafting_ingredient> &ingredients = recipe.get_ingredients();
    FT_ASSERT_EQ(1u, ingredients.size());
    FT_ASSERT_EQ(item_id, ingredients[0].get_item_id());
    FT_ASSERT_EQ(count, ingredients[0].get_count());
    FT_ASSERT_EQ(rarity, ingredients[0].get_rarity());
    return (1);
}

static int assert_loadout_entry_values(const ft_loadout_blueprint &loadout, size_t index,
        int slot, int item_id, int quantity)
{
    const ft_vector<ft_loadout_entry> &entries = loadout.get_entries();
    FT_ASSERT_EQ(slot, entries[index].get_slot());
    FT_ASSERT_EQ(item_id, entries[index].get_item_id());
    FT_ASSERT_EQ(quantity, entries[index].get_quantity());
    return (1);
}

FT_TEST(test_catalog_register_and_fetch_item, "register and fetch item definition")
{
    ft_data_catalog catalog;
    ft_item_definition definition(1, 2, 5, 3, 4, 6, 7);
    ft_item_definition fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.register_item_definition(definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.fetch_item_definition(1, fetched));
    assert_item_definition_equals(fetched, 1, 2, 5, 3, 4, 6, 7);
    return (1);
}

FT_TEST(test_catalog_fetch_missing_item, "fetch missing item returns error")
{
    ft_data_catalog catalog;
    ft_item_definition definition;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, catalog.fetch_item_definition(99, definition));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, catalog.get_error());
    return (1);
}

FT_TEST(test_catalog_recipe_copy_isolated, "recipe copy remains isolated")
{
    ft_data_catalog catalog;
    ft_vector<ft_crafting_ingredient> ingredients;
    ft_crafting_ingredient ingredient;
    ft_recipe_blueprint recipe(5, 42, ingredients);
    ft_recipe_blueprint first_fetch;
    ft_recipe_blueprint second_fetch;

    populate_ingredient(ingredient, 2, 3, 1);
    ingredients.push_back(ingredient);
    recipe.set_ingredients(ingredients);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.register_recipe(recipe));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.fetch_recipe(5, first_fetch));
    assert_single_ingredient(first_fetch, 2, 3, 1);

    ft_vector<ft_crafting_ingredient> &first_ingredients = first_fetch.get_ingredients();
    first_ingredients[0].set_count(9);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.fetch_recipe(5, second_fetch));
    assert_single_ingredient(second_fetch, 2, 3, 1);
    return (1);
}

FT_TEST(test_catalog_loadout_copy_isolated, "loadout copy remains isolated")
{
    ft_data_catalog catalog;
    ft_vector<ft_loadout_entry> entries;
    ft_loadout_entry entry;
    ft_loadout_blueprint loadout(7, entries);
    ft_loadout_blueprint first_fetch;
    ft_loadout_blueprint second_fetch;

    populate_loadout_entry(entry, 0, 10, 2);
    entries.push_back(entry);
    populate_loadout_entry(entry, 1, 11, 1);
    entries.push_back(entry);
    loadout.set_entries(entries);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.register_loadout(loadout));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.fetch_loadout(7, first_fetch));
    const ft_vector<ft_loadout_entry> &first_entries = first_fetch.get_entries();
    FT_ASSERT_EQ(2u, first_entries.size());
    assert_loadout_entry_values(first_fetch, 0, 0, 10, 2);
    assert_loadout_entry_values(first_fetch, 1, 1, 11, 1);

    ft_vector<ft_loadout_entry> &first_mutable_entries = first_fetch.get_entries();
    first_mutable_entries[0].set_quantity(5);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, catalog.fetch_loadout(7, second_fetch));
    assert_loadout_entry_values(second_fetch, 0, 0, 10, 2);
    assert_loadout_entry_values(second_fetch, 1, 1, 11, 1);
    return (1);
}

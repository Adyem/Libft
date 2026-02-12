#include "../test_internal.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_item.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

int test_inventory_slots(void)
{
    ft_inventory inventory(4);
    ft_sharedptr<ft_item> bulky(new ft_item());
    bulky->set_item_id(1);
    bulky->set_max_stack(1);
    bulky->set_stack_size(1);
    bulky->set_width(2);
    bulky->set_height(2);
    if (inventory.add_item(bulky) != FT_ERR_SUCCESS)
        return (0);
    if (inventory.get_used() != 4)
        return (0);
    ft_sharedptr<ft_item> small(new ft_item());
    small->set_item_id(2);
    small->set_max_stack(1);
    small->set_stack_size(1);
    if (inventory.add_item(small) != FT_ERR_FULL)
        return (0);
    return (1);
}

int test_inventory_count(void)
{
    ft_inventory inv(5);
    ft_sharedptr<ft_item> potion(new ft_item());
    potion->set_item_id(1);
    potion->set_max_stack(10);
    potion->set_stack_size(7);
    inv.add_item(potion);

    ft_sharedptr<ft_item> more(new ft_item());
    more->set_item_id(1);
    more->set_max_stack(10);
    more->set_stack_size(4);
    inv.add_item(more);

    if (!inv.has_item(1) || inv.count_item(1) != 11)
        return (0);
    if (inv.has_item(2) || inv.count_item(2) != 0)
        return (0);
    return (1);
}

int test_inventory_full(void)
{
    ft_inventory inv(1);
    ft_sharedptr<ft_item> item(new ft_item());
    item->set_item_id(1);
    item->set_max_stack(5);
    item->set_stack_size(5);
    if (inv.is_full())
        return (0);
    if (inv.add_item(item) != FT_ERR_SUCCESS)
        return (0);
    if (!inv.is_full())
        return (0);
    return (1);
}

FT_TEST(test_game_inventory_remove_clears_usage, "ft_inventory::remove_item frees slots and weight")
{
    ft_inventory inventory(4);
    ft_sharedptr<ft_item> shield(new ft_item());

    shield->set_item_id(7);
    shield->set_max_stack(3);
    shield->set_stack_size(2);
    shield->set_width(2);
    shield->set_height(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(shield));
    FT_ASSERT_EQ((size_t)4, inventory.get_used());
    FT_ASSERT_EQ(2, inventory.get_current_weight());

    inventory.remove_item(0);
    FT_ASSERT_EQ((size_t)0, inventory.get_used());
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(0, inventory.count_item(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_game_inventory_merges_into_existing_stack, "ft_inventory merges items before opening a new slot")
{
    ft_inventory inventory(3);
    ft_sharedptr<ft_item> arrows(new ft_item());
    ft_sharedptr<ft_item> refill(new ft_item());

    arrows->set_item_id(12);
    arrows->set_max_stack(5);
    arrows->set_stack_size(4);
    arrows->set_width(1);
    arrows->set_height(1);
    refill->set_item_id(12);
    refill->set_max_stack(5);
    refill->set_stack_size(4);
    refill->set_width(1);
    refill->set_height(1);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(arrows));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(refill));
    FT_ASSERT_EQ(8, inventory.count_item(12));
    Pair<int, ft_sharedptr<ft_item> > *first_slot = inventory.get_items().find(0);
    Pair<int, ft_sharedptr<ft_item> > *second_slot = inventory.get_items().find(1);
    FT_ASSERT_NEQ(first_slot, inventory.get_items().end());
    FT_ASSERT_NEQ(second_slot, inventory.get_items().end());
    FT_ASSERT_EQ(5, first_slot->value->get_stack_size());
    FT_ASSERT_EQ(3, second_slot->value->get_stack_size());
    FT_ASSERT_EQ(8, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_game_inventory_rarity_tracking, "ft_inventory counts rarity stacks")
{
    ft_inventory inventory(3);
    ft_sharedptr<ft_item> gem(new ft_item());
    ft_sharedptr<ft_item> ore(new ft_item());

    gem->set_item_id(10);
    gem->set_rarity(2);
    gem->set_max_stack(10);
    gem->set_stack_size(3);
    ore->set_item_id(11);
    ore->set_rarity(2);
    ore->set_max_stack(10);
    ore->set_stack_size(4);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(gem));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(ore));
    FT_ASSERT_EQ(7, inventory.count_rarity(2));
    FT_ASSERT_EQ(true, inventory.has_rarity(2));
    FT_ASSERT_EQ(false, inventory.has_rarity(5));
    return (1);
}

FT_TEST(test_game_inventory_resize_updates_capacity, "ft_inventory::resize adjusts capacity")
{
    ft_inventory inventory(1);

    FT_ASSERT_EQ((size_t)1, inventory.get_capacity());
    inventory.resize(3);
    FT_ASSERT_EQ((size_t)3, inventory.get_capacity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}

FT_TEST(test_game_inventory_add_item_respects_weight_limit, "ft_inventory::add_item enforces weight limits")
{
    ft_inventory inventory(5, 3);
    ft_sharedptr<ft_item> heavy(new ft_item());

    heavy->set_item_id(4);
    heavy->set_max_stack(10);
    heavy->set_stack_size(4);
    heavy->set_width(1);
    heavy->set_height(1);
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.add_item(heavy));
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.get_error());
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
    return (1);
}

FT_TEST(test_game_inventory_rejects_null_item, "ft_inventory::add_item handles null items")
{
    ft_inventory inventory(2);
    ft_sharedptr<ft_item> none;

    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, inventory.add_item(none));
    FT_ASSERT_EQ((size_t)0, inventory.get_used());
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, inventory.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    return (1);
}

FT_TEST(test_game_inventory_splits_large_stack, "ft_inventory splits stacks that exceed max stack size")
{
    ft_inventory inventory(5);
    ft_sharedptr<ft_item> arrows(new ft_item());

    arrows->set_item_id(9);
    arrows->set_max_stack(5);
    arrows->set_stack_size(7);
    arrows->set_width(1);
    arrows->set_height(1);

    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(arrows));
    FT_ASSERT_EQ(7, inventory.count_item(9));
    FT_ASSERT_EQ((size_t)2, inventory.get_used());
    FT_ASSERT_EQ(7, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_inventory_full_addition_preserves_items, "ft_inventory leaves items unchanged on capacity error")
{
    ft_inventory inventory(1);
    ft_sharedptr<ft_item> potion(new ft_item());
    ft_sharedptr<ft_item> elixir(new ft_item());

    potion->set_item_id(3);
    potion->set_max_stack(2);
    potion->set_stack_size(1);
    potion->set_width(1);
    potion->set_height(1);
    elixir->set_item_id(4);
    elixir->set_max_stack(2);
    elixir->set_stack_size(1);
    elixir->set_width(1);
    elixir->set_height(1);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(potion));
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.add_item(elixir));
    FT_ASSERT_EQ((size_t)1, inventory.get_used());
    FT_ASSERT_EQ(1, inventory.count_item(3));
    FT_ASSERT_EQ(0, inventory.count_item(4));
    FT_ASSERT_EQ(1, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.get_error());
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
    return (1);
}

FT_TEST(test_game_inventory_remove_missing_slot_noops, "ft_inventory ignores removal of nonexistent slots")
{
    ft_inventory inventory(2);
    ft_sharedptr<ft_item> potion(new ft_item());

    potion->set_item_id(6);
    potion->set_max_stack(3);
    potion->set_stack_size(2);
    potion->set_width(1);
    potion->set_height(1);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.add_item(potion));
    FT_ASSERT_EQ(2, inventory.get_current_weight());
    inventory.remove_item(5);
    Pair<int, ft_sharedptr<ft_item> > *slot = inventory.get_items().find(0);
    FT_ASSERT_NEQ(slot, inventory.get_items().end());
    FT_ASSERT_EQ(2, slot->value->get_stack_size());
    FT_ASSERT_EQ(2, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, inventory.get_error());
    return (1);
}


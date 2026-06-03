#include "../test_internal.hpp"
#include "../../Modules/Game/game_hooks.hpp"
#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include <cstdint>
#include <cstdlib>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int32_t initialize_hook_metadata(ft_game_hook_metadata &metadata,
    const char *hook_identifier, const char *listener_name,
    const char *description, const char *argument_contract)
{
    int32_t error_code;

    error_code = metadata.hook_identifier.initialize(hook_identifier);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = metadata.listener_name.initialize(listener_name);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = metadata.description.initialize(description);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = metadata.argument_contract.initialize(argument_contract);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

struct game_hooks_test_backend_state
{
    void    *allocations[64];
    ft_size_t    sizes[64];
    ft_size_t    allocation_slots;
    ft_size_t    allocation_attempt_count;
    ft_size_t    fail_on_attempt_count;
};

static void game_hooks_test_backend_state_initialize(
    game_hooks_test_backend_state *state)
{
    ft_size_t index;

    index = 0;
    while (index < static_cast<ft_size_t>(64))
    {
        state->allocations[index] = ft_nullptr;
        state->sizes[index] = 0;
        index += 1;
    }
    state->allocation_slots = 0;
    state->allocation_attempt_count = 0;
    state->fail_on_attempt_count = 0;
    return ;
}

static int32_t game_hooks_test_backend_find_slot(
    const game_hooks_test_backend_state *state, const void *memory_pointer)
{
    ft_size_t index;

    if (memory_pointer == ft_nullptr)
        return (-1);
    index = 0;
    while (index < state->allocation_slots)
    {
        if (state->allocations[index] == memory_pointer)
            return (static_cast<int32_t>(index));
        index += 1;
    }
    return (-1);
}

static void *game_hooks_test_backend_allocate(ft_size_t size, void *user_data)
{
    game_hooks_test_backend_state *state;
    void *memory_pointer;

    state = static_cast<game_hooks_test_backend_state *>(user_data);
    state->allocation_attempt_count += 1;
    if (state->allocation_attempt_count == state->fail_on_attempt_count)
        return (ft_nullptr);
    memory_pointer = std::malloc(size);
    if (memory_pointer == ft_nullptr)
        return (ft_nullptr);
    if (state->allocation_slots >= static_cast<ft_size_t>(64))
    {
        std::free(memory_pointer);
        return (ft_nullptr);
    }
    state->allocations[state->allocation_slots] = memory_pointer;
    state->sizes[state->allocation_slots] = size;
    state->allocation_slots += 1;
    return (memory_pointer);
}

static void *game_hooks_test_backend_reallocate(void *memory_pointer,
    ft_size_t size, void *user_data)
{
    game_hooks_test_backend_state *state;
    int32_t slot_index;
    void *new_memory_pointer;

    state = static_cast<game_hooks_test_backend_state *>(user_data);
    if (memory_pointer == ft_nullptr)
        return (game_hooks_test_backend_allocate(size, user_data));
    if (size == 0)
    {
        slot_index = game_hooks_test_backend_find_slot(state, memory_pointer);
        if (slot_index >= 0)
        {
            state->allocations[slot_index]
                = state->allocations[state->allocation_slots - 1];
            state->sizes[slot_index] = state->sizes[state->allocation_slots - 1];
            state->allocations[state->allocation_slots - 1] = ft_nullptr;
            state->sizes[state->allocation_slots - 1] = 0;
            state->allocation_slots -= 1;
        }
        std::free(memory_pointer);
        return (ft_nullptr);
    }
    slot_index = game_hooks_test_backend_find_slot(state, memory_pointer);
    new_memory_pointer = std::realloc(memory_pointer, size);
    if (new_memory_pointer == ft_nullptr)
        return (ft_nullptr);
    if (slot_index >= 0)
    {
        state->allocations[slot_index] = new_memory_pointer;
        state->sizes[slot_index] = size;
    }
    return (new_memory_pointer);
}

static void game_hooks_test_backend_deallocate(void *memory_pointer,
    void *user_data)
{
    game_hooks_test_backend_state *state;
    int32_t slot_index;

    state = static_cast<game_hooks_test_backend_state *>(user_data);
    if (memory_pointer == ft_nullptr)
        return ;
    slot_index = game_hooks_test_backend_find_slot(state, memory_pointer);
    if (slot_index >= 0)
    {
        state->allocations[slot_index]
            = state->allocations[state->allocation_slots - 1];
        state->sizes[slot_index] = state->sizes[state->allocation_slots - 1];
        state->allocations[state->allocation_slots - 1] = ft_nullptr;
        state->sizes[state->allocation_slots - 1] = 0;
        state->allocation_slots -= 1;
    }
    std::free(memory_pointer);
    return ;
}

static ft_size_t game_hooks_test_backend_get_allocation_size(
    const void *memory_pointer, void *user_data)
{
    game_hooks_test_backend_state *state;
    int32_t slot_index;

    state = static_cast<game_hooks_test_backend_state *>(user_data);
    slot_index = game_hooks_test_backend_find_slot(state, memory_pointer);
    if (slot_index < 0)
        return (0);
    return (state->sizes[slot_index]);
}

static ft_bool game_hooks_test_backend_owns_allocation(
    const void *memory_pointer, void *user_data)
{
    game_hooks_test_backend_state *state;

    state = static_cast<game_hooks_test_backend_state *>(user_data);
    if (game_hooks_test_backend_find_slot(state, memory_pointer) < 0)
        return (FT_FALSE);
    return (FT_TRUE);
}

FT_TEST(test_game_hooks_invoke_callbacks)
{
    game_hooks hooks;
    game_character character;
    game_item item;
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    game_event event;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, event.initialize());
    int crafted_invocations;
    int damaged_invocations;
    int event_invocations;
    int invocation_counter;
    int high_priority_step;
    int legacy_step;
    int low_priority_step;

    crafted_invocations = 0;
    damaged_invocations = 0;
    event_invocations = 0;
    invocation_counter = 0;
    high_priority_step = 0;
    legacy_step = 0;
    low_priority_step = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    hooks.set_on_item_crafted(ft_function<void(game_character&, game_item&)>([&crafted_invocations, &invocation_counter, &legacy_step](game_character &character_ref, game_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        crafted_invocations = crafted_invocations + 1;
        invocation_counter = invocation_counter + 1;
        legacy_step = invocation_counter;
        return ;
    }));
    hooks.set_on_character_damaged(ft_function<void(game_character&, int, uint8_t)>([&damaged_invocations](game_character &character_ref, int damage, uint8_t type)
    {
        (void)character_ref;
        (void)damage;
        (void)type;
        damaged_invocations = damaged_invocations + 1;
        return ;
    }));
    hooks.set_on_event_triggered(ft_function<void(game_world&, game_event&)>([&event_invocations](game_world &world_ref, game_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        event_invocations = event_invocations + 1;
        return ;
    }));
    ft_game_hook_metadata high_metadata;
    ft_game_hook_metadata low_metadata;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, initialize_hook_metadata(high_metadata,
        ft_game_hook_item_crafted_identifier, "test.high",
        "High priority test listener", "game_character&,game_item&"));
    hooks.register_listener(high_metadata, 1500, ft_game_hook_make_character_item_adapter(ft_function<void(game_character&, game_item&)>([&invocation_counter, &high_priority_step](game_character &character_ref, game_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        invocation_counter = invocation_counter + 1;
        high_priority_step = invocation_counter;
        return ;
    })));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, initialize_hook_metadata(low_metadata,
        ft_game_hook_item_crafted_identifier, "test.low",
        "Low priority test listener", "game_character&,game_item&"));
    hooks.register_listener(low_metadata, 500, ft_game_hook_make_character_item_adapter(ft_function<void(game_character&, game_item&)>([&invocation_counter, &low_priority_step](game_character &character_ref, game_item &item_ref)
    {
        (void)character_ref;
        (void)item_ref;
        invocation_counter = invocation_counter + 1;
        low_priority_step = invocation_counter;
        return ;
    })));
    hooks.invoke_on_item_crafted(character, item);
    hooks.invoke_on_character_damaged(character, 5, static_cast<uint8_t>(2));
    hooks.invoke_on_event_triggered(world, event);
    FT_ASSERT_EQ(1, crafted_invocations);
    FT_ASSERT_EQ(1, damaged_invocations);
    FT_ASSERT_EQ(1, event_invocations);
    FT_ASSERT_EQ(1, high_priority_step);
    FT_ASSERT_EQ(2, legacy_step);
    FT_ASSERT_EQ(3, low_priority_step);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    return (1);
}

FT_TEST(test_game_hooks_legacy_callback_setter_failure_preserves_previous_callback)
{
    game_hooks hooks;
    game_character character;
    game_item item;
    int old_invocations;
    int new_invocations;
    ft_function<void(game_character&, game_item&)> callback_after_failure;
    game_hooks_test_backend_state backend_state;
    cma_backend_hooks backend_hooks;
    int32_t setter_error;

    old_invocations = 0;
    new_invocations = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, character.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.initialize());
    hooks.set_on_item_crafted(ft_function<void(game_character&, game_item&)>(
        [&old_invocations](game_character &character_ref, game_item &item_ref)
        {
            (void)character_ref;
            (void)item_ref;
            old_invocations = old_invocations + 1;
            return ;
        }));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    callback_after_failure = hooks.get_on_item_crafted();
    callback_after_failure(character, item);
    FT_ASSERT_EQ(1, old_invocations);
    FT_ASSERT_EQ(0, new_invocations);
    game_hooks_test_backend_state_initialize(&backend_state);
    backend_state.fail_on_attempt_count = static_cast<ft_size_t>(2);
    backend_hooks.allocate = &game_hooks_test_backend_allocate;
    backend_hooks.reallocate = &game_hooks_test_backend_reallocate;
    backend_hooks.deallocate = &game_hooks_test_backend_deallocate;
    backend_hooks.aligned_allocate = ft_nullptr;
    backend_hooks.get_allocation_size = &game_hooks_test_backend_get_allocation_size;
    backend_hooks.owns_allocation = &game_hooks_test_backend_owns_allocation;
    backend_hooks.user_data = &backend_state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_backend(&backend_hooks));
    hooks.set_on_item_crafted(ft_function<void(game_character&, game_item&)>(
        [&new_invocations](game_character &character_ref, game_item &item_ref)
        {
            (void)character_ref;
            (void)item_ref;
            new_invocations = new_invocations + 1;
            return ;
        }));
    setter_error = hooks.get_error();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_clear_backend());
    FT_ASSERT_NE(FT_ERR_SUCCESS, setter_error);
    callback_after_failure = hooks.get_on_item_crafted();
    callback_after_failure(character, item);
    FT_ASSERT_EQ(2, old_invocations);
    FT_ASSERT_EQ(0, new_invocations);
    hooks.invoke_on_item_crafted(character, item);
    FT_ASSERT_EQ(3, old_invocations);
    FT_ASSERT_EQ(0, new_invocations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    return (1);
}

FT_TEST(test_game_hooks_register_listener_allocation_failure_rolls_back)
{
    game_hooks hooks;
    ft_game_hook_metadata metadata;
    game_character character;
    game_item item;
    int crafted_invocations;
    int32_t register_result;
    int32_t destroy_result;
    ft_vector<ft_game_hook_metadata> *metadata_copy;
    game_hooks_test_backend_state backend_state;
    cma_backend_hooks backend_hooks;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, initialize_hook_metadata(metadata,
        ft_game_hook_item_crafted_identifier, "test.failure",
        "Allocation failure rollback test", "game_character&,game_item&"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    destroy_result = hooks.destroy();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destroy_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.initialize());
    crafted_invocations = 0;
    game_hooks_test_backend_state_initialize(&backend_state);
    backend_state.fail_on_attempt_count = static_cast<ft_size_t>(4);
    backend_hooks.allocate = &game_hooks_test_backend_allocate;
    backend_hooks.reallocate = &game_hooks_test_backend_reallocate;
    backend_hooks.deallocate = &game_hooks_test_backend_deallocate;
    backend_hooks.aligned_allocate = ft_nullptr;
    backend_hooks.get_allocation_size = &game_hooks_test_backend_get_allocation_size;
    backend_hooks.owns_allocation = &game_hooks_test_backend_owns_allocation;
    backend_hooks.user_data = &backend_state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_backend(&backend_hooks));
    hooks.register_listener(metadata, 1000,
        ft_game_hook_make_character_item_adapter(ft_function<void(game_character&, game_item&)>(
            [&crafted_invocations](game_character &character_ref, game_item &item_ref)
            {
                (void)character_ref;
                (void)item_ref;
                crafted_invocations = crafted_invocations + 1;
                return ;
            })));
    register_result = hooks.get_error();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_clear_backend());
    FT_ASSERT_NE(FT_ERR_SUCCESS, register_result);
    metadata_copy = hooks.get_catalog_metadata();
    FT_ASSERT(metadata_copy != ft_nullptr);
    if (metadata_copy != ft_nullptr)
    {
        FT_ASSERT_EQ(static_cast<ft_size_t>(0), metadata_copy->size());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, metadata_copy->destroy());
        delete metadata_copy;
    }
    hooks.invoke_on_item_crafted(character, item);
    FT_ASSERT_EQ(0, crafted_invocations);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hooks.get_error());
    return (1);
}

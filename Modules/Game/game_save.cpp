#include "game_character.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "game_quest.hpp"
#include "../Errno/errno.hpp"
#include "../JSon/json.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"
#include "game_achievement.hpp"
#include "game_buff.hpp"
#include "game_debuff.hpp"
#include "game_skill.hpp"
#include "game_upgrade.hpp"

json_group *serialize_character(const game_character &character);
json_group *serialize_inventory(const game_inventory &inventory);
json_group *serialize_equipment(const game_character &character);
json_group *serialize_quest(const game_quest &quest);

static void game_save_delete_item_handle(ft_sharedptr<game_item> *item) noexcept
{
    if (item == ft_nullptr)
        return ;
    (void)item->destroy();
    delete item;
    return ;
}

static int32_t add_item_field(json_group *group, const ft_string &key, int32_t value)
{
    json_item *json_item_ptr = json_create_item(key.c_str(), value);
    if (!json_item_ptr)
    {
        json_free_groups(group);
        return (FT_ERR_NO_MEMORY);
    }
    json_add_item_to_group(group, json_item_ptr);
    return (FT_ERR_SUCCESS);
}

static int32_t build_item_field_key(const ft_string &item_prefix,
    const char *suffix, ft_string &key) noexcept
{
    int32_t error_code;

    error_code = key.initialize(item_prefix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = key.append(suffix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

static int32_t serialize_item_fields(json_group *group, const game_item &item, const ft_string &item_prefix)
{
    ft_string key_max;
    if (build_item_field_key(item_prefix, "_max_stack", key_max) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_max, item.get_max_stack()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_current;
    if (build_item_field_key(item_prefix, "_stack_size", key_current) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_current, item.get_stack_size()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_id;
    if (build_item_field_key(item_prefix, "_id", key_id) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_id, item.get_item_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_width;
    if (build_item_field_key(item_prefix, "_width", key_width) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_width, item.get_width()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_height;
    if (build_item_field_key(item_prefix, "_height", key_height) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_height, item.get_height()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_id;
    if (build_item_field_key(item_prefix, "_mod1_id", key_mod1_id) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod1_id, item.get_modifier1_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_value;
    if (build_item_field_key(item_prefix, "_mod1_value", key_mod1_value) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod1_value, item.get_modifier1_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_id;
    if (build_item_field_key(item_prefix, "_mod2_id", key_mod2_id) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod2_id, item.get_modifier2_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_value;
    if (build_item_field_key(item_prefix, "_mod2_value", key_mod2_value) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod2_value, item.get_modifier2_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_id;
    if (build_item_field_key(item_prefix, "_mod3_id", key_mod3_id) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod3_id, item.get_modifier3_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_value;
    if (build_item_field_key(item_prefix, "_mod3_value", key_mod3_value) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod3_value, item.get_modifier3_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_id;
    if (build_item_field_key(item_prefix, "_mod4_id", key_mod4_id) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod4_id, item.get_modifier4_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_value;
    if (build_item_field_key(item_prefix, "_mod4_value", key_mod4_value) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (add_item_field(group, key_mod4_value, item.get_modifier4_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    return (FT_ERR_SUCCESS);
}

json_group *serialize_inventory(const game_inventory &inventory)
{
    json_group *group = json_create_json_group("inventory");
    if (!group)
    {
        return (ft_nullptr);
    }
    ft_bool has_error = FT_FALSE;
    do
    {
        json_item *capacity_item = json_create_item("capacity", static_cast<int32_t>(inventory.get_capacity()));
        if (!capacity_item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, capacity_item);
        json_item *weight_limit_item = json_create_item("weight_limit", inventory.get_weight_limit());
        if (!weight_limit_item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, weight_limit_item);
        json_item *current_weight_item = json_create_item("current_weight", inventory.get_current_weight());
        if (!current_weight_item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, current_weight_item);
        json_item *used_slots_item = json_create_item("used_slots", static_cast<int32_t>(inventory.get_used()));
        if (!used_slots_item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, used_slots_item);
        ft_size_t item_count = inventory.get_items().size();
        json_item *count_item = json_create_item("item_count", static_cast<int32_t>(item_count));
        if (!count_item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, count_item);
        const Pair<int32_t, ft_sharedptr<game_item> > *items_end = inventory.get_items().end();
        const Pair<int32_t, ft_sharedptr<game_item> > *item_start = items_end;
        if (item_count > 0)
        {
            if (!items_end)
            {
                has_error = FT_TRUE;
                break ;
            }
            item_start = items_end - item_count;
        }
        ft_size_t item_index = 0;
        while (item_index < item_count)
        {
            char item_index_string[32];

            std::snprintf(item_index_string, sizeof(item_index_string), "%d",
                static_cast<int32_t>(item_index));
            ft_string item_prefix;
            if (item_prefix.initialize("item_") != FT_ERR_SUCCESS)
            {
                has_error = FT_TRUE;
                break ;
            }
            item_prefix += item_index_string;
            if (!item_start[item_index].value)
            {
                has_error = FT_TRUE;
                break ;
            }
            if (serialize_item_fields(group, *item_start[item_index].value, item_prefix) != FT_ERR_SUCCESS)
            {
                has_error = FT_TRUE;
                break ;
            }
            item_index++;
        }
        if (has_error)
            break ;
    } while (0);
    if (has_error)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    return (group);
}

json_group *serialize_equipment(const game_character &character)
{
    json_group *group = json_create_json_group("equipment");
    if (!group)
    {
        return (ft_nullptr);
    }
    ft_sharedptr<game_item> *head;
    int32_t head_present_value;

    head = character.get_equipped_item(EQUIP_HEAD);
    if (head == ft_nullptr)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*head)
        head_present_value = 1;
    else
        head_present_value = 0;
    json_item *present = json_create_item("head_present", head_present_value);
    if (!present)
    {
        game_save_delete_item_handle(head);
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string head_prefix;
    if (head_prefix.initialize("head") != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(head);
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*head && serialize_item_fields(group, **head, head_prefix) != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(head);
        json_free_groups(group);
        return (ft_nullptr);
    }
    game_save_delete_item_handle(head);
    ft_sharedptr<game_item> *chest;
    int32_t chest_present_value;

    chest = character.get_equipped_item(EQUIP_CHEST);
    if (chest == ft_nullptr)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*chest)
        chest_present_value = 1;
    else
        chest_present_value = 0;
    present = json_create_item("chest_present", chest_present_value);
    if (!present)
    {
        game_save_delete_item_handle(chest);
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string chest_prefix;
    if (chest_prefix.initialize("chest") != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(chest);
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*chest && serialize_item_fields(group, **chest, chest_prefix) != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(chest);
        json_free_groups(group);
        return (ft_nullptr);
    }
    game_save_delete_item_handle(chest);
    ft_sharedptr<game_item> *weapon;
    int32_t weapon_present_value;

    weapon = character.get_equipped_item(EQUIP_WEAPON);
    if (weapon == ft_nullptr)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*weapon)
        weapon_present_value = 1;
    else
        weapon_present_value = 0;
    present = json_create_item("weapon_present", weapon_present_value);
    if (!present)
    {
        game_save_delete_item_handle(weapon);
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string weapon_prefix;
    if (weapon_prefix.initialize("weapon") != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(weapon);
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (*weapon && serialize_item_fields(group, **weapon, weapon_prefix) != FT_ERR_SUCCESS)
    {
        game_save_delete_item_handle(weapon);
        json_free_groups(group);
        return (ft_nullptr);
    }
    game_save_delete_item_handle(weapon);
    return (group);
}

json_group *serialize_quest(const game_quest &quest)
{
    json_group *group = json_create_json_group("quest");
    if (!group)
    {
        return (ft_nullptr);
    }
    ft_bool has_error = FT_FALSE;
    do
    {
        json_item *item = json_create_item("id", quest.get_id());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("phases", quest.get_phases());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("current_phase", quest.get_current_phase());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("description", quest.get_description().c_str());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("objective", quest.get_objective().c_str());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("reward_experience", quest.get_reward_experience());
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        ft_size_t item_count = quest.get_reward_items().size();
        item = json_create_item("reward_item_count", static_cast<int32_t>(item_count));
        if (!item)
        {
            has_error = FT_TRUE;
            break ;
        }
        json_add_item_to_group(group, item);
        const ft_vector<ft_sharedptr<game_item> > &reward_items = quest.get_reward_items();
        const ft_sharedptr<game_item> *item_start = reward_items.begin();
        if (item_count > 0 && !item_start)
        {
            has_error = FT_TRUE;
            break ;
        }
        ft_size_t item_index = 0;
        while (item_index < item_count)
        {
            char item_index_string[32];

            std::snprintf(item_index_string, sizeof(item_index_string), "%d",
                static_cast<int32_t>(item_index));
            ft_string item_prefix;
            if (item_prefix.initialize("reward_item_") != FT_ERR_SUCCESS)
            {
                has_error = FT_TRUE;
                break ;
            }
            item_prefix += item_index_string;
            if (!item_start[item_index])
            {
                has_error = FT_TRUE;
                break ;
            }
            if (serialize_item_fields(group, *item_start[item_index], item_prefix) != FT_ERR_SUCCESS)
            {
                has_error = FT_TRUE;
                break ;
            }
            item_index++;
        }
        if (has_error)
            break ;
    } while (0);
    if (has_error)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    return (group);
}

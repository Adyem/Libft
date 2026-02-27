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

json_group *serialize_character(const ft_character &character);
json_group *serialize_inventory(const ft_inventory &inventory);
json_group *serialize_equipment(const ft_character &character);
json_group *serialize_quest(const ft_quest &quest);

static int add_item_field(json_group *group, const ft_string &key, int value)
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

static int serialize_item_fields(json_group *group, const ft_item &item, const ft_string &item_prefix)
{
    ft_string key_max = item_prefix;
    key_max += "_max_stack";
    if (add_item_field(group, key_max, item.get_max_stack()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_current = item_prefix;
    key_current += "_stack_size";
    if (add_item_field(group, key_current, item.get_stack_size()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_id = item_prefix;
    key_id += "_id";
    if (add_item_field(group, key_id, item.get_item_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_width = item_prefix;
    key_width += "_width";
    if (add_item_field(group, key_width, item.get_width()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_height = item_prefix;
    key_height += "_height";
    if (add_item_field(group, key_height, item.get_height()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_id = item_prefix;
    key_mod1_id += "_mod1_id";
    if (add_item_field(group, key_mod1_id, item.get_modifier1_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_value = item_prefix;
    key_mod1_value += "_mod1_value";
    if (add_item_field(group, key_mod1_value, item.get_modifier1_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_id = item_prefix;
    key_mod2_id += "_mod2_id";
    if (add_item_field(group, key_mod2_id, item.get_modifier2_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_value = item_prefix;
    key_mod2_value += "_mod2_value";
    if (add_item_field(group, key_mod2_value, item.get_modifier2_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_id = item_prefix;
    key_mod3_id += "_mod3_id";
    if (add_item_field(group, key_mod3_id, item.get_modifier3_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_value = item_prefix;
    key_mod3_value += "_mod3_value";
    if (add_item_field(group, key_mod3_value, item.get_modifier3_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_id = item_prefix;
    key_mod4_id += "_mod4_id";
    if (add_item_field(group, key_mod4_id, item.get_modifier4_id()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_value = item_prefix;
    key_mod4_value += "_mod4_value";
    if (add_item_field(group, key_mod4_value, item.get_modifier4_value()) != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    return (FT_ERR_SUCCESS);
}

json_group *serialize_inventory(const ft_inventory &inventory)
{
    json_group *group = json_create_json_group("inventory");
    if (!group)
    {
        return (ft_nullptr);
    }
    bool has_error = false;
    do
    {
        json_item *capacity_item = json_create_item("capacity", static_cast<int>(inventory.get_capacity()));
        if (!capacity_item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, capacity_item);
        json_item *weight_limit_item = json_create_item("weight_limit", inventory.get_weight_limit());
        if (!weight_limit_item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, weight_limit_item);
        json_item *current_weight_item = json_create_item("current_weight", inventory.get_current_weight());
        if (!current_weight_item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, current_weight_item);
        json_item *used_slots_item = json_create_item("used_slots", static_cast<int>(inventory.get_used()));
        if (!used_slots_item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, used_slots_item);
        size_t item_count = inventory.get_items().size();
        json_item *count_item = json_create_item("item_count", static_cast<int>(item_count));
        if (!count_item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, count_item);
        const Pair<int, ft_sharedptr<ft_item> > *items_end = inventory.get_items().end();
        const Pair<int, ft_sharedptr<ft_item> > *item_start = items_end;
        if (item_count > 0)
        {
            if (!items_end)
            {
                has_error = true;
                break ;
            }
            item_start = items_end - item_count;
        }
        size_t item_index = 0;
        while (item_index < item_count)
        {
            char item_index_string[32];

            std::snprintf(item_index_string, sizeof(item_index_string), "%d",
                static_cast<int>(item_index));
            ft_string item_prefix;
            if (item_prefix.initialize("item_") != FT_ERR_SUCCESS)
            {
                has_error = true;
                break ;
            }
            item_prefix += item_index_string;
            if (!item_start[item_index].value)
            {
                has_error = true;
                break ;
            }
            if (serialize_item_fields(group, *item_start[item_index].value, item_prefix) != FT_ERR_SUCCESS)
            {
                has_error = true;
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

json_group *serialize_equipment(const ft_character &character)
{
    json_group *group = json_create_json_group("equipment");
    if (!group)
    {
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> head = character.get_equipped_item(EQUIP_HEAD);
    int head_present_value;

    if (head)
        head_present_value = 1;
    else
        head_present_value = 0;
    json_item *present = json_create_item("head_present", head_present_value);
    if (!present)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string head_prefix;
    if (head_prefix.initialize("head") != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (head && serialize_item_fields(group, *head, head_prefix) != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> chest = character.get_equipped_item(EQUIP_CHEST);
    int chest_present_value;

    if (chest)
        chest_present_value = 1;
    else
        chest_present_value = 0;
    present = json_create_item("chest_present", chest_present_value);
    if (!present)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string chest_prefix;
    if (chest_prefix.initialize("chest") != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (chest && serialize_item_fields(group, *chest, chest_prefix) != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> weapon = character.get_equipped_item(EQUIP_WEAPON);
    int weapon_present_value;

    if (weapon)
        weapon_present_value = 1;
    else
        weapon_present_value = 0;
    present = json_create_item("weapon_present", weapon_present_value);
    if (!present)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    ft_string weapon_prefix;
    if (weapon_prefix.initialize("weapon") != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    if (weapon && serialize_item_fields(group, *weapon, weapon_prefix) != FT_ERR_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    return (group);
}

json_group *serialize_quest(const ft_quest &quest)
{
    json_group *group = json_create_json_group("quest");
    if (!group)
    {
        return (ft_nullptr);
    }
    bool has_error = false;
    do
    {
        json_item *item = json_create_item("id", quest.get_id());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("phases", quest.get_phases());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("current_phase", quest.get_current_phase());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("description", quest.get_description().c_str());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("objective", quest.get_objective().c_str());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("reward_experience", quest.get_reward_experience());
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        size_t item_count = quest.get_reward_items().size();
        item = json_create_item("reward_item_count", static_cast<int>(item_count));
        if (!item)
        {
            has_error = true;
            break ;
        }
        json_add_item_to_group(group, item);
        const ft_vector<ft_sharedptr<ft_item> > &reward_items = quest.get_reward_items();
        const ft_sharedptr<ft_item> *item_start = reward_items.begin();
        if (item_count > 0 && !item_start)
        {
            has_error = true;
            break ;
        }
        size_t item_index = 0;
        while (item_index < item_count)
        {
            char item_index_string[32];

            std::snprintf(item_index_string, sizeof(item_index_string), "%d",
                static_cast<int>(item_index));
            ft_string item_prefix;
            if (item_prefix.initialize("reward_item_") != FT_ERR_SUCCESS)
            {
                has_error = true;
                break ;
            }
            item_prefix += item_index_string;
            if (!item_start[item_index])
            {
                has_error = true;
                break ;
            }
            if (serialize_item_fields(group, *item_start[item_index], item_prefix) != FT_ERR_SUCCESS)
            {
                has_error = true;
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

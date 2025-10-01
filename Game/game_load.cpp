#include "game_character.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "game_quest.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Template/shared_ptr.hpp"

int deserialize_character(ft_character &character, json_group *group);
int deserialize_inventory(ft_inventory &inventory, json_group *group);
int deserialize_equipment(ft_character &character, json_group *group);
int deserialize_quest(ft_quest &quest, json_group *group);

static int parse_item_field(json_group *group, const ft_string &key, int &out_value)
{
    json_item *json_item_ptr = json_find_item(group, key.c_str());
    if (!json_item_ptr)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    out_value = ft_atoi(json_item_ptr->value);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

static int build_item_from_group(ft_item &item, json_group *group, const ft_string &item_prefix)
{
    int value;
    ft_string key_max = item_prefix;
    key_max += "_max_stack";
    if (parse_item_field(group, key_max, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_max_stack(value);
    ft_string key_current = item_prefix;
    key_current += "_stack_size";
    if (parse_item_field(group, key_current, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_stack_size(value);
    ft_string key_id = item_prefix;
    key_id += "_id";
    if (parse_item_field(group, key_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_item_id(value);
    ft_string key_width = item_prefix;
    key_width += "_width";
    if (parse_item_field(group, key_width, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_width(value);
    ft_string key_height = item_prefix;
    key_height += "_height";
    if (parse_item_field(group, key_height, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_height(value);
    ft_string key_mod1_id = item_prefix;
    key_mod1_id += "_mod1_id";
    if (parse_item_field(group, key_mod1_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier1_id(value);
    ft_string key_mod1_value = item_prefix;
    key_mod1_value += "_mod1_value";
    if (parse_item_field(group, key_mod1_value, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier1_value(value);
    ft_string key_mod2_id = item_prefix;
    key_mod2_id += "_mod2_id";
    if (parse_item_field(group, key_mod2_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier2_id(value);
    ft_string key_mod2_value = item_prefix;
    key_mod2_value += "_mod2_value";
    if (parse_item_field(group, key_mod2_value, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier2_value(value);
    ft_string key_mod3_id = item_prefix;
    key_mod3_id += "_mod3_id";
    if (parse_item_field(group, key_mod3_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier3_id(value);
    ft_string key_mod3_value = item_prefix;
    key_mod3_value += "_mod3_value";
    if (parse_item_field(group, key_mod3_value, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier3_value(value);
    ft_string key_mod4_id = item_prefix;
    key_mod4_id += "_mod4_id";
    if (parse_item_field(group, key_mod4_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier4_id(value);
    ft_string key_mod4_value = item_prefix;
    key_mod4_value += "_mod4_value";
    if (parse_item_field(group, key_mod4_value, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_modifier4_value(value);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int deserialize_inventory(ft_inventory &inventory, json_group *group)
{
    json_item *capacity_item = json_find_item(group, "capacity");
    if (!capacity_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    inventory.resize(ft_atoi(capacity_item->value));
    json_item *weight_item = json_find_item(group, "weight_limit");
    if (!weight_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    inventory.set_weight_limit(ft_atoi(weight_item->value));
    json_item *cur_weight_item = json_find_item(group, "current_weight");
    if (!cur_weight_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int serialized_weight = ft_atoi(cur_weight_item->value);
    json_item *used_slots_item = json_find_item(group, "used_slots");
    if (!used_slots_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int serialized_slots = ft_atoi(used_slots_item->value);
    inventory.set_current_weight(0);
    inventory.set_used_slots(0);
    inventory.get_items().clear();
    json_item *count_item = json_find_item(group, "item_count");
    if (!count_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int item_count = ft_atoi(count_item->value);
    int item_index = 0;
    int loop_error = ER_SUCCESS;
    while (item_index < item_count)
    {
        char *item_index_string = cma_itoa(item_index);
        if (!item_index_string)
        {
            ft_errno = JSON_MALLOC_FAIL;
            return (JSON_MALLOC_FAIL);
        }
        ft_string item_prefix = "item_";
        item_prefix += item_index_string;
        cma_free(item_index_string);
        ft_item item_temp;
        if (build_item_from_group(item_temp, group, item_prefix) != ER_SUCCESS)
        {
            loop_error = GAME_GENERAL_ERROR;
            break ;
        }
        ft_sharedptr<ft_item> item(new ft_item(item_temp));
        if (!item)
        {
            ft_errno = JSON_MALLOC_FAIL;
            return (JSON_MALLOC_FAIL);
        }
        if (inventory.add_item(item) != ER_SUCCESS)
        {
            loop_error = inventory.get_error();
            break ;
        }
        item_index++;
    }
    if (loop_error != ER_SUCCESS)
    {
        ft_errno = loop_error;
        return (loop_error);
    }
    inventory.set_current_weight(serialized_weight);
    inventory.set_used_slots(serialized_slots);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int deserialize_equipment(ft_character &character, json_group *group)
{
    json_item *present = json_find_item(group, "head_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item_temp;
        if (build_item_from_group(item_temp, group, "head") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        ft_sharedptr<ft_item> item(new ft_item(item_temp));
        if (character.equip_item(EQUIP_HEAD, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_HEAD);
    present = json_find_item(group, "chest_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item_temp;
        if (build_item_from_group(item_temp, group, "chest") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        ft_sharedptr<ft_item> item(new ft_item(item_temp));
        if (character.equip_item(EQUIP_CHEST, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_CHEST);
    present = json_find_item(group, "weapon_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item_temp;
        if (build_item_from_group(item_temp, group, "weapon") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        ft_sharedptr<ft_item> item(new ft_item(item_temp));
        if (character.equip_item(EQUIP_WEAPON, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_WEAPON);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int deserialize_quest(ft_quest &quest, json_group *group)
{
    json_item *item = json_find_item(group, "id");
    if (item)
        quest.set_id(ft_atoi(item->value));
    item = json_find_item(group, "phases");
    if (item)
        quest.set_phases(ft_atoi(item->value));
    item = json_find_item(group, "current_phase");
    if (item)
        quest.set_current_phase(ft_atoi(item->value));
    item = json_find_item(group, "description");
    if (item)
    {
        ft_string description = item->value;
        quest.set_description(description);
    }
    item = json_find_item(group, "objective");
    if (item)
    {
        ft_string objective = item->value;
        quest.set_objective(objective);
    }
    item = json_find_item(group, "reward_experience");
    if (item)
        quest.set_reward_experience(ft_atoi(item->value));
    json_item *count_item = json_find_item(group, "reward_item_count");
    if (count_item)
    {
        int reward_count = ft_atoi(count_item->value);
        int reward_index = 0;
        quest.get_reward_items().clear();
        while (reward_index < reward_count)
        {
            char *index_string = cma_itoa(reward_index);
            if (!index_string)
            {
                ft_errno = JSON_MALLOC_FAIL;
                return (JSON_MALLOC_FAIL);
            }
            ft_string prefix = "reward_item_";
            prefix += index_string;
            cma_free(index_string);
            ft_item reward_temp;
            if (build_item_from_group(reward_temp, group, prefix) != ER_SUCCESS)
                return (GAME_GENERAL_ERROR);
            ft_sharedptr<ft_item> reward(new ft_item(reward_temp));
            if (!reward)
            {
                ft_errno = JSON_MALLOC_FAIL;
                return (JSON_MALLOC_FAIL);
            }
            quest.get_reward_items().push_back(reward);
            reward_index++;
        }
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}


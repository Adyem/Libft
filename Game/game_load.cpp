#include "world.hpp"
#include "character.hpp"
#include "inventory.hpp"
#include "item.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"

int deserialize_character(ft_character &character, json_group *group);
int deserialize_world(ft_world &world, json_group *group);
int deserialize_inventory(ft_inventory &inventory, json_group *group);
int deserialize_equipment(ft_character &character, json_group *group);

static int parse_item_field(json_group *group, const ft_string &key, int &out_value)
{
    json_item *json_item_ptr = json_find_item(group, key.c_str());
    if (!json_item_ptr)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    out_value = ft_atoi(json_item_ptr->value);
    return (ER_SUCCESS);
}

int deserialize_world(ft_world &world, json_group *group)
{
    json_item *count_item = json_find_item(group, "event_count");
    if (!count_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int event_count = ft_atoi(count_item->value);
    int event_index = 0;
    while (event_index < event_count)
    {
        char *event_index_string = cma_itoa(event_index);
        if (!event_index_string)
            return (JSON_MALLOC_FAIL);
        ft_string key_id = "event_";
        key_id += event_index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += event_index_string;
        key_duration += "_duration";
        cma_free(event_index_string);
        json_item *id_item = json_find_item(group, key_id.c_str());
        json_item *duration_item = json_find_item(group, key_duration.c_str());
        if (!id_item || !duration_item)
        {
            ft_errno = GAME_GENERAL_ERROR;
            return (GAME_GENERAL_ERROR);
        }
        ft_event event;
        event.set_id(ft_atoi(id_item->value));
        event.set_duration(ft_atoi(duration_item->value));
        world.get_events().insert(event.get_id(), event);
        event_index++;
    }
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
    key_current += "_current_stack";
    if (parse_item_field(group, key_current, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_current_stack(value);
    ft_string key_id = item_prefix;
    key_id += "_id";
    if (parse_item_field(group, key_id, value) != ER_SUCCESS)
        return (GAME_GENERAL_ERROR);
    item.set_item_id(value);
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
    inventory.get_items().clear();
    json_item *count_item = json_find_item(group, "item_count");
    if (!count_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int item_count = ft_atoi(count_item->value);
    int item_index = 0;
    while (item_index < item_count)
    {
        char *item_index_string = cma_itoa(item_index);
        if (!item_index_string)
            return (JSON_MALLOC_FAIL);
        ft_string item_prefix = "item_";
        item_prefix += item_index_string;
        cma_free(item_index_string);
        ft_item item;
        if (build_item_from_group(item, group, item_prefix) != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        if (inventory.add_item(item) != ER_SUCCESS)
            return (inventory.get_error());
        item_index++;
    }
    return (ER_SUCCESS);
}

int deserialize_equipment(ft_character &character, json_group *group)
{
    json_item *present = json_find_item(group, "head_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item;
        if (build_item_from_group(item, group, "head") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        if (character.equip_item(EQUIP_HEAD, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_HEAD);
    present = json_find_item(group, "chest_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item;
        if (build_item_from_group(item, group, "chest") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        if (character.equip_item(EQUIP_CHEST, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_CHEST);
    present = json_find_item(group, "weapon_present");
    if (present && ft_atoi(present->value) == 1)
    {
        ft_item item;
        if (build_item_from_group(item, group, "weapon") != ER_SUCCESS)
            return (GAME_GENERAL_ERROR);
        if (character.equip_item(EQUIP_WEAPON, item) != ER_SUCCESS)
            return (character.get_error());
    }
    else
        character.unequip_item(EQUIP_WEAPON);
    return (ER_SUCCESS);
}


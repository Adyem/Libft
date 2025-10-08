#include "game_character.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "game_quest.hpp"
#include "../Errno/errno.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"

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
    return (ER_SUCCESS);
}

static int serialize_item_fields(json_group *group, const ft_item &item, const ft_string &item_prefix)
{
    ft_string key_max = item_prefix;
    key_max += "_max_stack";
    if (add_item_field(group, key_max, item.get_max_stack()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_current = item_prefix;
    key_current += "_stack_size";
    if (add_item_field(group, key_current, item.get_stack_size()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_id = item_prefix;
    key_id += "_id";
    if (add_item_field(group, key_id, item.get_item_id()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_width = item_prefix;
    key_width += "_width";
    if (add_item_field(group, key_width, item.get_width()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_height = item_prefix;
    key_height += "_height";
    if (add_item_field(group, key_height, item.get_height()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_id = item_prefix;
    key_mod1_id += "_mod1_id";
    if (add_item_field(group, key_mod1_id, item.get_modifier1_id()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod1_value = item_prefix;
    key_mod1_value += "_mod1_value";
    if (add_item_field(group, key_mod1_value, item.get_modifier1_value()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_id = item_prefix;
    key_mod2_id += "_mod2_id";
    if (add_item_field(group, key_mod2_id, item.get_modifier2_id()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod2_value = item_prefix;
    key_mod2_value += "_mod2_value";
    if (add_item_field(group, key_mod2_value, item.get_modifier2_value()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_id = item_prefix;
    key_mod3_id += "_mod3_id";
    if (add_item_field(group, key_mod3_id, item.get_modifier3_id()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod3_value = item_prefix;
    key_mod3_value += "_mod3_value";
    if (add_item_field(group, key_mod3_value, item.get_modifier3_value()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_id = item_prefix;
    key_mod4_id += "_mod4_id";
    if (add_item_field(group, key_mod4_id, item.get_modifier4_id()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    ft_string key_mod4_value = item_prefix;
    key_mod4_value += "_mod4_value";
    if (add_item_field(group, key_mod4_value, item.get_modifier4_value()) != ER_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    return (ER_SUCCESS);
}

json_group *serialize_inventory(const ft_inventory &inventory)
{
    json_group *group = json_create_json_group("inventory");
    if (!group)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    bool has_error = false;
    int error_code = ER_SUCCESS;
    do
    {
        json_item *capacity_item = json_create_item("capacity", static_cast<int>(inventory.get_capacity()));
        if (!capacity_item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, capacity_item);
        json_item *weight_limit_item = json_create_item("weight_limit", inventory.get_weight_limit());
        if (!weight_limit_item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, weight_limit_item);
        json_item *current_weight_item = json_create_item("current_weight", inventory.get_current_weight());
        if (!current_weight_item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, current_weight_item);
        json_item *used_slots_item = json_create_item("used_slots", static_cast<int>(inventory.get_used()));
        if (!used_slots_item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, used_slots_item);
        size_t item_count = inventory.get_items().size();
        json_item *count_item = json_create_item("item_count", static_cast<int>(item_count));
        if (!count_item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, count_item);
        const Pair<int, ft_sharedptr<ft_item> > *items_end = inventory.get_items().end();
        const Pair<int, ft_sharedptr<ft_item> > *item_start = items_end;
        if (item_count > 0)
        {
            if (!items_end)
            {
                error_code = FT_ERR_GAME_GENERAL_ERROR;
                has_error = true;
                break;
            }
            item_start = items_end - item_count;
        }
        size_t item_index = 0;
        while (item_index < item_count)
        {
            char *item_index_string = cma_itoa(static_cast<int>(item_index));
            if (!item_index_string)
            {
                error_code = FT_ERR_NO_MEMORY;
                has_error = true;
                break;
            }
            ft_string item_prefix = "item_";
            item_prefix += item_index_string;
            cma_free(item_index_string);
            if (!item_start[item_index].value)
            {
                error_code = FT_ERR_GAME_GENERAL_ERROR;
                has_error = true;
                break;
            }
            if (serialize_item_fields(group, *item_start[item_index].value, item_prefix) != ER_SUCCESS)
            {
                error_code = FT_ERR_NO_MEMORY;
                has_error = true;
                break;
            }
            item_index++;
        }
        if (has_error)
            break;
    } while (0);
    if (has_error)
    {
        json_free_groups(group);
        ft_errno = error_code;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (group);
}

json_group *serialize_equipment(const ft_character &character)
{
    json_group *group = json_create_json_group("equipment");
    if (!group)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> head = character.get_equipped_item(EQUIP_HEAD);
    json_item *present = json_create_item("head_present", head ? 1 : 0);
    if (!present)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    if (head && serialize_item_fields(group, *head, "head") != ER_SUCCESS)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> chest = character.get_equipped_item(EQUIP_CHEST);
    present = json_create_item("chest_present", chest ? 1 : 0);
    if (!present)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    if (chest && serialize_item_fields(group, *chest, "chest") != ER_SUCCESS)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_sharedptr<ft_item> weapon = character.get_equipped_item(EQUIP_WEAPON);
    present = json_create_item("weapon_present", weapon ? 1 : 0);
    if (!present)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_add_item_to_group(group, present);
    if (weapon && serialize_item_fields(group, *weapon, "weapon") != ER_SUCCESS)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (group);
}

json_group *serialize_quest(const ft_quest &quest)
{
    json_group *group = json_create_json_group("quest");
    if (!group)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    bool has_error = false;
    int error_code = ER_SUCCESS;
    do
    {
        json_item *item = json_create_item("id", quest.get_id());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("phases", quest.get_phases());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("current_phase", quest.get_current_phase());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("description", quest.get_description().c_str());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("objective", quest.get_objective().c_str());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        item = json_create_item("reward_experience", quest.get_reward_experience());
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        size_t item_count = quest.get_reward_items().size();
        item = json_create_item("reward_item_count", static_cast<int>(item_count));
        if (!item)
        {
            error_code = FT_ERR_NO_MEMORY;
            has_error = true;
            break;
        }
        json_add_item_to_group(group, item);
        const ft_vector<ft_sharedptr<ft_item> > &reward_items = quest.get_reward_items();
        const ft_sharedptr<ft_item> *item_start = reward_items.begin();
        if (item_count > 0 && !item_start)
        {
            error_code = FT_ERR_GAME_GENERAL_ERROR;
            has_error = true;
            break;
        }
        size_t item_index = 0;
        while (item_index < item_count)
        {
            char *item_index_string = cma_itoa(static_cast<int>(item_index));
            if (!item_index_string)
            {
                error_code = FT_ERR_NO_MEMORY;
                has_error = true;
                break;
            }
            ft_string item_prefix = "reward_item_";
            item_prefix += item_index_string;
            cma_free(item_index_string);
            if (!item_start[item_index])
            {
                error_code = FT_ERR_GAME_GENERAL_ERROR;
                has_error = true;
                break;
            }
            if (serialize_item_fields(group, *item_start[item_index], item_prefix) != ER_SUCCESS)
            {
                error_code = FT_ERR_NO_MEMORY;
                has_error = true;
                break;
            }
            item_index++;
        }
        if (has_error)
            break;
    } while (0);
    if (has_error)
    {
        json_free_groups(group);
        ft_errno = error_code;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (group);
}


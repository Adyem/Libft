#include "game_character.hpp"
#include "../Errno/errno.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"

json_group *serialize_character(const ft_character &character)
{
    json_group *group = json_create_json_group("character");
    if (!group)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_item *item = json_create_item("hit_points", character.get_hit_points());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("physical_armor", character.get_physical_armor());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("magic_armor", character.get_magic_armor());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("current_physical_armor", character.get_current_physical_armor());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("current_magic_armor", character.get_current_magic_armor());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("damage_rule", character.get_damage_rule());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("might", character.get_might());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("agility", character.get_agility());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("endurance", character.get_endurance());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("reason", character.get_reason());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("insigh", character.get_insigh());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("presence", character.get_presence());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("coins", character.get_coins());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("valor", character.get_valor());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("experience", character.get_experience());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("x", character.get_x());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("y", character.get_y());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("z", character.get_z());
    if (!item)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    const ft_map<int, ft_skill> &skills = character.get_skills();
    if (skills.last_operation_error() != FT_ERR_SUCCESSS)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_GAME_GENERAL_ERROR);
        return (ft_nullptr);
    }
    json_item *count = json_create_item("skill_count", static_cast<int>(skills.size()));
    if (!count)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, count);
    size_t skill_index = 0;
    size_t skill_count = skills.size();
    const Pair<int, ft_skill> *skills_end = skills.end();
    if (skill_count > 0 && !skills_end)
    {
        json_free_groups(group);
        ft_global_error_stack_push(FT_ERR_GAME_GENERAL_ERROR);
        return (ft_nullptr);
    }
    const Pair<int, ft_skill> *skill_start = skills_end;
    if (skill_count > 0)
        skill_start = skills_end - skill_count;
    while (skill_index < skill_count)
    {
        char *skill_index_string = cma_itoa(static_cast<int>(skill_index));
        if (!skill_index_string)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        ft_string prefix = "skill_";
        prefix += skill_index_string;
        cma_free(skill_index_string);
        ft_string key = prefix;
        key += "_id";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_id());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_level";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_level());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_cooldown";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_cooldown());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_mod1";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_modifier1());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_mod2";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_modifier2());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_mod3";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_modifier3());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        key = prefix;
        key += "_mod4";
        item = json_create_item(key.c_str(), skill_start[skill_index].value.get_modifier4());
        if (!item)
        {
            json_free_groups(group);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item);
        skill_index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (group);
}

int deserialize_character(ft_character &character, json_group *group)
{
    json_item *item = json_find_item(group, "hit_points");
    if (item)
        character.set_hit_points(ft_atoi(item->value));
    item = json_find_item(group, "physical_armor");
    if (item)
        character.set_physical_armor(ft_atoi(item->value));
    item = json_find_item(group, "magic_armor");
    if (item)
        character.set_magic_armor(ft_atoi(item->value));
    item = json_find_item(group, "current_physical_armor");
    if (item)
        character.set_current_physical_armor(ft_atoi(item->value));
    item = json_find_item(group, "current_magic_armor");
    if (item)
        character.set_current_magic_armor(ft_atoi(item->value));
    item = json_find_item(group, "damage_rule");
    if (item)
        character.set_damage_rule(static_cast<uint8_t>(ft_atoi(item->value)));
    item = json_find_item(group, "might");
    if (item)
        character.set_might(ft_atoi(item->value));
    item = json_find_item(group, "agility");
    if (item)
        character.set_agility(ft_atoi(item->value));
    item = json_find_item(group, "endurance");
    if (item)
        character.set_endurance(ft_atoi(item->value));
    item = json_find_item(group, "reason");
    if (item)
        character.set_reason(ft_atoi(item->value));
    item = json_find_item(group, "insigh");
    if (item)
        character.set_insigh(ft_atoi(item->value));
    item = json_find_item(group, "presence");
    if (item)
        character.set_presence(ft_atoi(item->value));
    item = json_find_item(group, "coins");
    if (item)
        character.set_coins(ft_atoi(item->value));
    item = json_find_item(group, "valor");
    if (item)
        character.set_valor(ft_atoi(item->value));
    item = json_find_item(group, "experience");
    if (item)
        character.set_experience(ft_atoi(item->value));
    item = json_find_item(group, "x");
    if (item)
        character.set_x(ft_atoi(item->value));
    item = json_find_item(group, "y");
    if (item)
        character.set_y(ft_atoi(item->value));
    item = json_find_item(group, "z");
    if (item)
        character.set_z(ft_atoi(item->value));
    json_item *count_item = json_find_item(group, "skill_count");
    if (count_item)
    {
        int skill_count = ft_atoi(count_item->value);
        int skill_index = 0;
        while (skill_index < skill_count)
        {
            char *skill_index_string = cma_itoa(skill_index);
            if (!skill_index_string)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                return (FT_ERR_NO_MEMORY);
            }
            ft_string prefix = "skill_";
            prefix += skill_index_string;
            cma_free(skill_index_string);
            ft_string key = prefix;
            key += "_id";
            json_item *id_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_level";
            json_item *level_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_cooldown";
            json_item *cool_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_mod1";
            json_item *mod1_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_mod2";
            json_item *mod2_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_mod3";
            json_item *mod3_item = json_find_item(group, key.c_str());
            key = prefix;
            key += "_mod4";
            json_item *mod4_item = json_find_item(group, key.c_str());
            if (!id_item || !level_item || !cool_item || !mod1_item || !mod2_item || !mod3_item || !mod4_item)
            {
                ft_global_error_stack_push(FT_ERR_GAME_GENERAL_ERROR);
                return (FT_ERR_GAME_GENERAL_ERROR);
            }
            ft_skill skill;
            skill.set_id(ft_atoi(id_item->value));
            skill.set_level(ft_atoi(level_item->value));
            skill.set_cooldown(ft_atoi(cool_item->value));
            skill.set_modifier1(ft_atoi(mod1_item->value));
            skill.set_modifier2(ft_atoi(mod2_item->value));
            skill.set_modifier3(ft_atoi(mod3_item->value));
            skill.set_modifier4(ft_atoi(mod4_item->value));
            if (character.add_skill(skill) != FT_ERR_SUCCESSS)
            {
                int add_error = character.get_error();
                ft_global_error_stack_push(add_error);
                return (add_error);
            }
            skill_index++;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

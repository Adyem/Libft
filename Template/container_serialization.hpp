#ifndef FT_CONTAINER_SERIALIZATION_HPP
#define FT_CONTAINER_SERIALIZATION_HPP

#include "../Errno/errno.hpp"
#include "../JSon/json.hpp"
#include "../YAML/yaml.hpp"
#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Libft/libft.hpp"
#include <type_traits>
#include <limits>
#include <new>
#include <cstdlib>

    template <typename ElementType>
    int default_string_serializer(const ElementType &value, ft_string &output) noexcept
    {
        if constexpr (std::is_same<ElementType, ft_string>::value)
        {
            output = value;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        else if constexpr (std::is_same<ElementType, const char *>::value)
        {
            if (!value)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            output = value;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        else if constexpr (std::is_integral<ElementType>::value)
        {
            if constexpr (std::numeric_limits<ElementType>::is_signed)
            {
                if constexpr (std::numeric_limits<ElementType>::max() > std::numeric_limits<long>::max())
                {
                    ft_global_error_stack_push(FT_ERR_UNSUPPORTED_TYPE);
                    return (-1);
                }
                if constexpr (std::numeric_limits<ElementType>::min() < std::numeric_limits<long>::min())
                {
                    ft_global_error_stack_push(FT_ERR_UNSUPPORTED_TYPE);
                    return (-1);
                }
                ft_string serialized = ft_to_string(static_cast<long>(value));
                if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    int error_code = ft_string::last_operation_error();
                    if (error_code == FT_ERR_SUCCESSS)
                        error_code = FT_ERR_NO_MEMORY;
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
                output = serialized;
                if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    int error_code = ft_string::last_operation_error();
                    if (error_code == FT_ERR_SUCCESSS)
                        error_code = FT_ERR_NO_MEMORY;
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
                ft_global_error_stack_push(FT_ERR_SUCCESSS);
                return (0);
            }
            else
            {
                ft_string serialized = ft_to_string(static_cast<unsigned long long>(value));
                if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    int error_code = ft_string::last_operation_error();
                    if (error_code == FT_ERR_SUCCESSS)
                        error_code = FT_ERR_NO_MEMORY;
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
                output = serialized;
                if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    int error_code = ft_string::last_operation_error();
                    if (error_code == FT_ERR_SUCCESSS)
                        error_code = FT_ERR_NO_MEMORY;
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
                ft_global_error_stack_push(FT_ERR_SUCCESSS);
                return (0);
            }
        }
        else if constexpr (std::is_floating_point<ElementType>::value)
        {
            ft_string serialized = ft_to_string(static_cast<double>(value));
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            output = serialized;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        else
        {
            ft_global_error_stack_push(FT_ERR_UNSUPPORTED_TYPE);
            return (-1);
        }
    }

template <typename ElementType>
int default_string_deserializer(const char *value_string, ElementType &output) noexcept
    {
        if (!value_string)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        if constexpr (std::is_same<ElementType, ft_string>::value)
        {
            output = value_string;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        else if constexpr (std::is_integral<ElementType>::value)
        {
            long parsed = ft_atol(value_string);
            if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
                return (-1);
            if constexpr (std::numeric_limits<ElementType>::is_signed)
            {
                if (parsed < std::numeric_limits<ElementType>::min() || parsed > std::numeric_limits<ElementType>::max())
                {
                    ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                    return (-1);
                }
                output = static_cast<ElementType>(parsed);
                ft_global_error_stack_push(FT_ERR_SUCCESSS);
                return (0);
            }
            else
            {
                if (parsed < 0)
                {
                    ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                    return (-1);
                }
                unsigned long converted = static_cast<unsigned long>(parsed);
                if (converted > std::numeric_limits<ElementType>::max())
                {
                    ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                    return (-1);
                }
                output = static_cast<ElementType>(converted);
                ft_global_error_stack_push(FT_ERR_SUCCESSS);
                return (0);
            }
        }
        else if constexpr (std::is_floating_point<ElementType>::value)
        {
            ft_string input_copy = value_string;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                int error_code = ft_string::last_operation_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_NO_MEMORY;
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            char *end_pointer;
            const char *c_string = input_copy.c_str();
            if (!c_string)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_STATE);
                return (-1);
            }
            double parsed_value = std::strtod(c_string, &end_pointer);
            if (end_pointer == c_string)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            output = static_cast<ElementType>(parsed_value);
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        else
        {
            ft_global_error_stack_push(FT_ERR_UNSUPPORTED_TYPE);
            return (-1);
        }
    }

template <typename ElementType, typename Serializer>
int ft_vector_serialize_json(const ft_vector<ElementType> &values,
    const char *group_name,
    Serializer serializer,
    json_group *&output_group,
    const char *count_key = "count",
    const char *item_prefix = "item_") noexcept
{
    output_group = ft_nullptr;
    if (!group_name || !count_key || !item_prefix)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_group *group = json_create_json_group(group_name);
    if (!group)
        return (-1);
    size_t index = 0;
    size_t total_size = values.size();
    int total_error = ft_global_error_stack_peek_last_error();
    if (total_error != FT_ERR_SUCCESSS)
    {
        json_free_groups(group);
        ft_global_error_stack_push(total_error);
        return (-1);
    }
    while (index < total_size)
    {
        const ElementType &element = values[index];
        if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
        {
            int index_error = ft_global_error_stack_peek_last_error();
            json_free_groups(group);
            ft_global_error_stack_push(index_error);
            return (-1);
        }
        ft_string value_string;
        if (serializer(element, value_string) != 0)
        {
            json_free_groups(group);
            return (-1);
        }
        ft_string key_string(item_prefix);
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            json_free_groups(group);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        ft_string index_string = ft_to_string(static_cast<long>(index));
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            json_free_groups(group);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        key_string += index_string;
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            json_free_groups(group);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        json_item *item = json_create_item(key_string.c_str(), value_string.c_str());
        if (!item)
        {
            json_free_groups(group);
            return (-1);
        }
        json_add_item_to_group(group, item);
        index += 1;
    }
    json_item *count_item = json_create_item(count_key, static_cast<int>(total_size));
    if (!count_item)
    {
        json_free_groups(group);
        return (-1);
    }
    json_add_item_to_group(group, count_item);
    output_group = group;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
int ft_vector_serialize_json(const ft_vector<ElementType> &values,
    const char *group_name,
    json_group *&output_group,
    const char *count_key = "count",
    const char *item_prefix = "item_") noexcept
{
    return (ft_vector_serialize_json(values, group_name,
            default_string_serializer<ElementType>,
            output_group, count_key, item_prefix));
}

template <typename ElementType, typename Deserializer>
int ft_vector_deserialize_json(json_group *group,
    const char *count_key,
    const char *item_prefix,
    ft_vector<ElementType> &output,
    Deserializer deserializer) noexcept
{
    if (!group || !count_key || !item_prefix)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_item *count_item = json_find_item(group, count_key);
    if (!count_item)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        return (-1);
    }
    long expected_count = ft_atol(count_item->value);
    if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
        return (-1);
    if (expected_count < 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (-1);
    }
    ft_vector<ElementType> parsed_values;
    size_t index = 0;
    while (index < static_cast<size_t>(expected_count))
    {
        ft_string key_string(item_prefix);
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        ft_string index_string = ft_to_string(static_cast<long>(index));
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        key_string += index_string;
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            int error_code = ft_string::last_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        json_item *value_item = json_find_item(group, key_string.c_str());
        if (!value_item)
        {
            ft_global_error_stack_push(FT_ERR_NOT_FOUND);
            return (-1);
        }
        ElementType element;
        if (deserializer(value_item->value, element) != 0)
            return (-1);
        parsed_values.push_back(element);
        {
            int parsed_error = ft_global_error_stack_peek_last_error();
            if (parsed_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(parsed_error);
                return (-1);
            }
        }
        index += 1;
    }
    output = ft_move(parsed_values);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
int ft_vector_deserialize_json(json_group *group,
    ft_vector<ElementType> &output,
    const char *count_key = "count",
    const char *item_prefix = "item_") noexcept
{
    return (ft_vector_deserialize_json(group, count_key, item_prefix, output,
            default_string_deserializer<ElementType>));
}

template <typename ElementType, typename Serializer>
int ft_vector_serialize_yaml(const ft_vector<ElementType> &values,
    Serializer serializer,
    yaml_value *&output_value) noexcept
{
    output_value = ft_nullptr;
    yaml_value *list_value = new (std::nothrow) yaml_value();
    if (!list_value)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    list_value->set_type(YAML_LIST);
    {
        int list_error = ft_global_error_stack_peek_last_error();
        if (list_error != FT_ERR_SUCCESSS)
        {
            delete list_value;
            ft_global_error_stack_push(list_error);
            return (-1);
        }
    }
    size_t index = 0;
    size_t total_size = values.size();
    {
        int total_error = ft_global_error_stack_peek_last_error();
        if (total_error != FT_ERR_SUCCESSS)
        {
            delete list_value;
            ft_global_error_stack_push(total_error);
            return (-1);
        }
    }
    while (index < total_size)
    {
        const ElementType &element = values[index];
        {
            int element_error = ft_global_error_stack_peek_last_error();
            if (element_error != FT_ERR_SUCCESSS)
            {
                yaml_free(list_value);
                ft_global_error_stack_push(element_error);
                return (-1);
            }
        }
        yaml_value *entry = new (std::nothrow) yaml_value();
        if (!entry)
        {
            yaml_free(list_value);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (-1);
        }
        entry->set_type(YAML_SCALAR);
        {
            int entry_error = ft_global_error_stack_peek_last_error();
            if (entry_error != FT_ERR_SUCCESSS)
            {
                delete entry;
                yaml_free(list_value);
                ft_global_error_stack_push(entry_error);
                return (-1);
            }
        }
        ft_string serialized_value;
        if (serializer(element, serialized_value) != 0)
        {
            delete entry;
            yaml_free(list_value);
            return (-1);
        }
        entry->set_scalar(serialized_value);
        {
            int entry_error = ft_global_error_stack_peek_last_error();
            if (entry_error != FT_ERR_SUCCESSS)
            {
                delete entry;
                yaml_free(list_value);
                ft_global_error_stack_push(entry_error);
                return (-1);
            }
        }
        list_value->add_list_item(entry);
        {
            int list_error = ft_global_error_stack_peek_last_error();
            if (list_error != FT_ERR_SUCCESSS)
            {
                yaml_free(list_value);
                ft_global_error_stack_push(list_error);
                return (-1);
            }
        }
        index += 1;
    }
    output_value = list_value;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
int ft_vector_serialize_yaml(const ft_vector<ElementType> &values,
    yaml_value *&output_value) noexcept
{
    return (ft_vector_serialize_yaml(values,
            default_string_serializer<ElementType>,
            output_value));
}

template <typename ElementType, typename Deserializer>
int ft_vector_deserialize_yaml(const yaml_value &value,
    ft_vector<ElementType> &output,
    Deserializer deserializer) noexcept
{
    if (value.get_type() != YAML_LIST)
    {
        int type_error = ft_global_error_stack_peek_last_error();
        if (type_error == FT_ERR_SUCCESSS)
            type_error = FT_ERR_UNSUPPORTED_TYPE;
        ft_global_error_stack_push(type_error);
        return (-1);
    }
    const ft_vector<yaml_value*> &children = value.get_list();
    {
        int list_error = ft_global_error_stack_peek_last_error();
        if (list_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(list_error);
            return (-1);
        }
    }
    size_t total = children.size();
    {
        int children_error = ft_global_error_stack_peek_last_error();
        if (children_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(children_error);
            return (-1);
        }
    }
    ft_vector<ElementType> parsed;
    size_t index = 0;
    while (index < total)
    {
        yaml_value *child = children[index];
        if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
        {
            int child_list_error = ft_global_error_stack_peek_last_error();
            ft_global_error_stack_push(child_list_error);
            return (-1);
        }
        if (!child)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_STATE);
            return (-1);
        }
        if (child->get_type() != YAML_SCALAR)
        {
            int child_error = ft_global_error_stack_peek_last_error();
            if (child_error == FT_ERR_SUCCESSS)
                child_error = FT_ERR_UNSUPPORTED_TYPE;
            ft_global_error_stack_push(child_error);
            return (-1);
        }
        const ft_string &scalar = child->get_scalar();
        if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
        {
            int child_error = ft_global_error_stack_peek_last_error();
            ft_global_error_stack_push(child_error);
            return (-1);
        }
        ElementType element;
        if (deserializer(scalar.c_str(), element) != 0)
            return (-1);
        parsed.push_back(element);
        if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
        {
            int parsed_error = ft_global_error_stack_peek_last_error();
            ft_global_error_stack_push(parsed_error);
            return (-1);
        }
        index += 1;
    }
    output = ft_move(parsed);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
int ft_vector_deserialize_yaml(const yaml_value &value,
    ft_vector<ElementType> &output) noexcept
{
    return (ft_vector_deserialize_yaml(value, output,
            default_string_deserializer<ElementType>));
}

#endif

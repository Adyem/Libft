#ifndef FT_CONTAINER_SERIALIZATION_HPP
#define FT_CONTAINER_SERIALIZATION_HPP

#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../Basic/basic.hpp"
#include "../JSon/json.hpp"
#include "../YAML/yaml.hpp"
#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include <type_traits>
#include <limits>
#include <new>
#include <cstdlib>
#include <cerrno>
#include <cstdio>

template <typename ElementType>
int32_t default_string_serializer(const ElementType &value, ft_string &output) noexcept
{
    if constexpr (std::is_same<ElementType, ft_string>::value)
    {
        output = value;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    else if constexpr (std::is_same<ElementType, const char *>::value)
    {
        if (value == ft_nullptr)
            return (-1);
        output = value;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    else if constexpr (std::is_integral<ElementType>::value)
    {
        char number_buffer[64];
        int32_t format_result;

        if constexpr (std::numeric_limits<ElementType>::is_signed)
        {
            format_result = std::snprintf(number_buffer, sizeof(number_buffer),
                    "%" FT_PRId64, static_cast<int64_t>(value));
            if (format_result <= 0)
                return (-1);
            output = number_buffer;
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }
        else
        {
            format_result = std::snprintf(number_buffer, sizeof(number_buffer),
                    "%" FT_PRIu64, static_cast<uint64_t>(value));
            if (format_result <= 0)
                return (-1);
            output = number_buffer;
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }
    }
    else if constexpr (std::is_floating_point<ElementType>::value)
    {
        char number_buffer[128];
        int32_t format_result;

        format_result = std::snprintf(number_buffer, sizeof(number_buffer),
                "%.17g", static_cast<double>(value));
        if (format_result <= 0)
            return (-1);
        output = number_buffer;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    return (-1);
}

template <typename ElementType>
int32_t default_string_deserializer(const char *value_string, ElementType &output) noexcept
{
    if (value_string == ft_nullptr)
        return (-1);
    if constexpr (std::is_same<ElementType, ft_string>::value)
    {
        output = value_string;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    else if constexpr (std::is_integral<ElementType>::value)
    {
        char *end_pointer;

        errno = 0;
        if constexpr (std::numeric_limits<ElementType>::is_signed)
        {
            int64_t parsed_value;

            parsed_value = ft_strtol(value_string, &end_pointer, 10);
            if (end_pointer == value_string || end_pointer == ft_nullptr)
                return (-1);
            if (parsed_value < static_cast<int64_t>(std::numeric_limits<ElementType>::min()))
                return (-1);
            if (parsed_value > static_cast<int64_t>(std::numeric_limits<ElementType>::max()))
                return (-1);
            output = static_cast<ElementType>(parsed_value);
            return (0);
        }
        else
        {
            uint64_t parsed_value;

            parsed_value = ft_strtoul(value_string, &end_pointer, 10);
            if (end_pointer == value_string || end_pointer == ft_nullptr)
                return (-1);
            if (parsed_value > static_cast<uint64_t>(std::numeric_limits<ElementType>::max()))
                return (-1);
            output = static_cast<ElementType>(parsed_value);
            return (0);
        }
    }
    else if constexpr (std::is_floating_point<ElementType>::value)
    {
        char *end_pointer;
        double parsed_value;

        errno = 0;
        parsed_value = std::strtod(value_string, &end_pointer);
        if (end_pointer == value_string || errno == ERANGE)
            return (-1);
        output = static_cast<ElementType>(parsed_value);
        return (0);
    }
    return (-1);
}

template <typename ElementType, typename Serializer>
int32_t ft_vector_serialize_json(const ft_vector<ElementType> &values,
    const char *group_name,
    Serializer serializer,
    json_group *&output_group,
    const char *count_key = "count",
    const char *item_prefix = "item_") noexcept
{
    json_group *group;
    ft_size_t index;
    ft_size_t total_size;

    output_group = ft_nullptr;
    if (group_name == ft_nullptr || count_key == ft_nullptr || item_prefix == ft_nullptr)
        return (-1);
    group = json_create_json_group(group_name);
    if (group == ft_nullptr)
        return (-1);
    total_size = values.size();
    index = 0;
    while (index < total_size)
    {
        const ElementType &element = values[index];
        ft_string value_string;
        ft_string key_string(item_prefix);
        ft_string index_string;
        json_item *item;

        if (serializer(element, value_string) != 0)
        {
            json_free_groups(group);
            return (-1);
        }
        {
            char index_buffer[32];
            int32_t format_result;

            format_result = std::snprintf(index_buffer, sizeof(index_buffer),
                    "%" FT_PRIu64, static_cast<uint64_t>(index));
            if (format_result <= 0)
            {
                json_free_groups(group);
                return (-1);
            }
            index_string = index_buffer;
        }
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            json_free_groups(group);
            return (-1);
        }
        key_string += index_string;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            json_free_groups(group);
            return (-1);
        }
        item = json_create_item(key_string.c_str(), value_string.c_str());
        if (item == ft_nullptr)
        {
            json_free_groups(group);
            return (-1);
        }
        json_add_item_to_group(group, item);
        ++index;
    }
    {
        json_item *count_item;

        count_item = json_create_item(count_key, static_cast<int32_t>(total_size));
        if (count_item == ft_nullptr)
        {
            json_free_groups(group);
            return (-1);
        }
        json_add_item_to_group(group, count_item);
    }
    output_group = group;
    return (0);
}

template <typename ElementType>
int32_t ft_vector_serialize_json(const ft_vector<ElementType> &values,
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
int32_t ft_vector_deserialize_json(json_group *group,
    const char *count_key,
    const char *item_prefix,
    ft_vector<ElementType> &output,
    Deserializer deserializer) noexcept
{
    json_item *count_item;
    int64_t expected_count;
    ft_size_t index;
    ft_vector<ElementType> parsed_values;

    if (group == ft_nullptr || count_key == ft_nullptr || item_prefix == ft_nullptr)
        return (-1);
    count_item = json_find_item(group, count_key);
    if (count_item == ft_nullptr || count_item->value == ft_nullptr)
        return (-1);
    errno = 0;
    expected_count = ft_strtol(count_item->value, ft_nullptr, 10);
    if (expected_count < 0)
        return (-1);
    index = 0;
    while (index < static_cast<ft_size_t>(expected_count))
    {
        ft_string key_string(item_prefix);
        ft_string index_string;
        json_item *value_item;
        ElementType element;

        {
            char index_buffer[32];
            int32_t format_result;

            format_result = std::snprintf(index_buffer, sizeof(index_buffer),
                    "%" FT_PRIu64, static_cast<uint64_t>(index));
            if (format_result <= 0)
                return (-1);
            index_string = index_buffer;
        }
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        key_string += index_string;
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (-1);
        value_item = json_find_item(group, key_string.c_str());
        if (value_item == ft_nullptr || value_item->value == ft_nullptr)
            return (-1);
        if (deserializer(value_item->value, element) != 0)
            return (-1);
        parsed_values.push_back(element);
        ++index;
    }
    output = ft_move(parsed_values);
    return (0);
}

template <typename ElementType>
int32_t ft_vector_deserialize_json(json_group *group,
    ft_vector<ElementType> &output,
    const char *count_key = "count",
    const char *item_prefix = "item_") noexcept
{
    return (ft_vector_deserialize_json(group, count_key, item_prefix, output,
            default_string_deserializer<ElementType>));
}

template <typename ElementType, typename Serializer>
int32_t ft_vector_serialize_yaml(const ft_vector<ElementType> &values,
    Serializer serializer,
    yaml_value *&output_value) noexcept
{
    yaml_value *list_value;
    ft_size_t index;
    ft_size_t total_size;

    output_value = ft_nullptr;
    list_value = new (std::nothrow) yaml_value();
    if (list_value == ft_nullptr)
        return (-1);
    list_value->set_type(YAML_LIST);
    total_size = values.size();
    index = 0;
    while (index < total_size)
    {
        const ElementType &element = values[index];
        yaml_value *entry;
        ft_string serialized_value;

        entry = new (std::nothrow) yaml_value();
        if (entry == ft_nullptr)
        {
            yaml_free(list_value);
            return (-1);
        }
        entry->set_type(YAML_SCALAR);
        if (serializer(element, serialized_value) != 0)
        {
            delete entry;
            yaml_free(list_value);
            return (-1);
        }
        entry->set_scalar(serialized_value);
        list_value->add_list_item(entry);
        ++index;
    }
    output_value = list_value;
    return (0);
}

template <typename ElementType>
int32_t ft_vector_serialize_yaml(const ft_vector<ElementType> &values,
    yaml_value *&output_value) noexcept
{
    return (ft_vector_serialize_yaml(values,
            default_string_serializer<ElementType>,
            output_value));
}

template <typename ElementType, typename Deserializer>
int32_t ft_vector_deserialize_yaml(const yaml_value &value,
    ft_vector<ElementType> &output,
    Deserializer deserializer) noexcept
{
    const ft_vector<yaml_value*> &children = value.get_list();
    ft_size_t total;
    ft_size_t index;
    ft_vector<ElementType> parsed;

    if (value.get_type() != YAML_LIST)
        return (-1);
    total = children.size();
    index = 0;
    while (index < total)
    {
        yaml_value *child;
        const ft_string &scalar;
        ElementType element;

        child = children[index];
        if (child == ft_nullptr)
            return (-1);
        if (child->get_type() != YAML_SCALAR)
            return (-1);
        scalar = child->get_scalar();
        if (deserializer(scalar.c_str(), element) != 0)
            return (-1);
        parsed.push_back(element);
        ++index;
    }
    output = ft_move(parsed);
    return (0);
}

template <typename ElementType>
int32_t ft_vector_deserialize_yaml(const yaml_value &value,
    ft_vector<ElementType> &output) noexcept
{
    return (ft_vector_deserialize_yaml(value, output,
            default_string_deserializer<ElementType>));
}

#endif

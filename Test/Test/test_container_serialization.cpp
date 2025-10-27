#include "../../Template/container_serialization.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../YAML/yaml.hpp"
#include "../../JSon/json.hpp"
#include "../../CMA/CMA.hpp"
#include <cstring>

FT_TEST(test_vector_json_serialization_round_trip,
    "ft_vector JSON serialization round trips integral values")
{
    ft_vector<int> numbers;

    numbers.push_back(10);
    FT_ASSERT_EQ(ER_SUCCESS, numbers.get_error());
    numbers.push_back(-4);
    FT_ASSERT_EQ(ER_SUCCESS, numbers.get_error());
    json_group *serialized = ft_nullptr;

    FT_ASSERT_EQ(0, ft_vector_serialize_json(numbers, "numbers", serialized));
    FT_ASSERT(serialized != ft_nullptr);
    json_item *count_item = json_find_item(serialized, "count");

    FT_ASSERT(count_item != ft_nullptr);
    FT_ASSERT_EQ(2L, ft_atol(count_item->value));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    json_item *first_item = json_find_item(serialized, "item_0");

    FT_ASSERT(first_item != ft_nullptr);
    FT_ASSERT(strcmp(first_item->value, "10") == 0);
    json_item *second_item = json_find_item(serialized, "item_1");

    FT_ASSERT(second_item != ft_nullptr);
    FT_ASSERT(strcmp(second_item->value, "-4") == 0);
    ft_vector<int> parsed;

    FT_ASSERT_EQ(0, ft_vector_deserialize_json(serialized, parsed));
    FT_ASSERT_EQ(2U, parsed.size());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(10, parsed[0]);
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    FT_ASSERT_EQ(-4, parsed[1]);
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    json_free_groups(serialized);
    return (1);
}

struct ft_serialized_entry
{
    ft_string name;
    int quantity;
};

static int ft_entry_serializer(const ft_serialized_entry &entry, ft_string &output) noexcept
{
    output = entry.name;
    if (output.get_error() != ER_SUCCESS)
        return (-1);
    ft_string separator("#");

    if (separator.get_error() != ER_SUCCESS)
        return (-1);
    output += separator;
    if (output.get_error() != ER_SUCCESS)
        return (-1);
    ft_string quantity_string = ft_to_string(static_cast<long>(entry.quantity));

    if (quantity_string.get_error() != ER_SUCCESS)
        return (-1);
    output += quantity_string;
    if (output.get_error() != ER_SUCCESS)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

static int ft_entry_deserializer(const char *value_string, ft_serialized_entry &entry) noexcept
{
    if (!value_string)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    const char *separator = ft_strchr(value_string, '#');

    if (!separator)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    size_t name_length = static_cast<size_t>(separator - value_string);
    ft_string name_buffer;

    name_buffer.assign(value_string, name_length);
    if (name_buffer.get_error() != ER_SUCCESS)
        return (-1);
    entry.name = name_buffer;
    if (entry.name.get_error() != ER_SUCCESS)
        return (-1);
    const char *quantity_part = separator + 1;

    entry.quantity = ft_atol(quantity_part);
    if (ft_errno != ER_SUCCESS)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

FT_TEST(test_vector_json_serialization_custom_type,
    "ft_vector JSON serialization accepts custom serializers")
{
    ft_serialized_entry first;
    ft_serialized_entry second;

    first.name = "apples";
    FT_ASSERT_EQ(ER_SUCCESS, first.name.get_error());
    first.quantity = 5;
    second.name = "oranges";
    FT_ASSERT_EQ(ER_SUCCESS, second.name.get_error());
    second.quantity = 9;
    ft_vector<ft_serialized_entry> entries;

    entries.push_back(first);
    FT_ASSERT_EQ(ER_SUCCESS, entries.get_error());
    entries.push_back(second);
    FT_ASSERT_EQ(ER_SUCCESS, entries.get_error());
    json_group *serialized = ft_nullptr;

    FT_ASSERT_EQ(0, ft_vector_serialize_json(entries, "inventory",
            ft_entry_serializer, serialized, "count", "entry_"));
    FT_ASSERT(serialized != ft_nullptr);
    ft_vector<ft_serialized_entry> parsed;

    FT_ASSERT_EQ(0, ft_vector_deserialize_json(serialized, "count", "entry_",
            parsed, ft_entry_deserializer));
    FT_ASSERT_EQ(2U, parsed.size());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(parsed[0].name == "apples");
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    FT_ASSERT_EQ(5, parsed[0].quantity);
    FT_ASSERT(parsed[1].name == "oranges");
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    FT_ASSERT_EQ(9, parsed[1].quantity);
    json_free_groups(serialized);
    return (1);
}

FT_TEST(test_vector_yaml_serialization_round_trip,
    "ft_vector YAML serialization round trips integral values")
{
    ft_vector<int> numbers;

    numbers.push_back(3);
    FT_ASSERT_EQ(ER_SUCCESS, numbers.get_error());
    numbers.push_back(7);
    FT_ASSERT_EQ(ER_SUCCESS, numbers.get_error());
    yaml_value *serialized = ft_nullptr;

    FT_ASSERT_EQ(0, ft_vector_serialize_yaml(numbers, serialized));
    FT_ASSERT(serialized != ft_nullptr);
    FT_ASSERT_EQ(YAML_LIST, serialized->get_type());
    FT_ASSERT_EQ(ER_SUCCESS, serialized->get_error());
    const ft_vector<yaml_value*> &children = serialized->get_list();

    FT_ASSERT_EQ(ER_SUCCESS, serialized->get_error());
    FT_ASSERT_EQ(2U, children.size());
    FT_ASSERT_EQ(ER_SUCCESS, children.get_error());
    FT_ASSERT(children[0] != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, children.get_error());
    FT_ASSERT(children[1] != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, children.get_error());
    FT_ASSERT(children[0]->get_scalar() == "3");
    FT_ASSERT_EQ(ER_SUCCESS, children[0]->get_error());
    FT_ASSERT(children[1]->get_scalar() == "7");
    FT_ASSERT_EQ(ER_SUCCESS, children[1]->get_error());
    ft_vector<int> parsed;

    FT_ASSERT_EQ(0, ft_vector_deserialize_yaml(*serialized, parsed));
    FT_ASSERT_EQ(2U, parsed.size());
    FT_ASSERT_EQ(3, parsed[0]);
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    FT_ASSERT_EQ(7, parsed[1]);
    FT_ASSERT_EQ(ER_SUCCESS, parsed.get_error());
    yaml_free(serialized);
    return (1);
}

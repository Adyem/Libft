#include "../test_internal.hpp"
#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <cstring>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

struct yaml_serializer_character
{
    const char *name;
    const char *role;
    ft_bool matched;
};

static int32_t yaml_serializer_add_scalar(yaml_value &root,
    const char *key_text, const char *value_text) noexcept
{
    yaml_value *value;
    ft_string key;
    ft_string scalar;
    int32_t operation_error;

    value = new (std::nothrow) yaml_value();
    if (value == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    operation_error = value->initialize();
    if (operation_error != FT_ERR_SUCCESS)
    {
        delete value;
        return (operation_error);
    }
    operation_error = key.initialize(key_text);
    if (operation_error != FT_ERR_SUCCESS)
    {
        delete value;
        return (operation_error);
    }
    operation_error = scalar.initialize(value_text);
    if (operation_error != FT_ERR_SUCCESS)
    {
        delete value;
        return (operation_error);
    }
    value->set_scalar(scalar);
    root.add_map_item(key, value);
    return (FT_ERR_SUCCESS);
}

static int32_t yaml_serializer_write_character(
    yaml_value &root, void *user_data) noexcept
{
    yaml_serializer_character *character;
    int32_t operation_error;

    character = static_cast<yaml_serializer_character *>(user_data);
    if (character == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    root.set_type(YAML_MAP);
    operation_error = yaml_serializer_add_scalar(root, "name", character->name);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = yaml_serializer_add_scalar(root, "role", character->role);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    return (FT_ERR_SUCCESS);
}

static int32_t yaml_serializer_read_character(
    const yaml_value &root, void *user_data) noexcept
{
    yaml_serializer_character *character;
    ft_string name_key;
    ft_string role_key;
    const yaml_value *name_value;
    const yaml_value *role_value;

    character = static_cast<yaml_serializer_character *>(user_data);
    if (character == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (root.get_type() != YAML_MAP)
        return (FT_ERR_INVALID_STATE);
    if (name_key.initialize("name") != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (role_key.initialize("role") != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    name_value = root.get_map().at(name_key);
    role_value = root.get_map().at(role_key);
    if (name_value == ft_nullptr || role_value == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    if (std::strcmp(name_value->get_scalar().c_str(), character->name) != 0)
        return (FT_ERR_INVALID_STATE);
    if (std::strcmp(role_value->get_scalar().c_str(), character->role) != 0)
        return (FT_ERR_INVALID_STATE);
    character->matched = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_yaml_serializer_to_string_and_from_string)
{
    yaml_serializer_character input_character;
    yaml_serializer_character output_character;
    ft_string serialized_content;

    input_character.name = "Ada";
    input_character.role = "wizard";
    input_character.matched = FT_FALSE;
    output_character.name = "Ada";
    output_character.role = "wizard";
    output_character.matched = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, serialized_content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, yaml_serialize_to_string(
            yaml_serializer_write_character, &input_character,
            serialized_content));
    FT_ASSERT(std::strstr(serialized_content.c_str(), "name: Ada")
        != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, yaml_deserialize_from_string(
            serialized_content, yaml_serializer_read_character,
            &output_character));
    FT_ASSERT_EQ(FT_TRUE, output_character.matched);
    return (1);
}

FT_TEST(test_yaml_serializer_rejects_null_callbacks)
{
    ft_string serialized_content;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, serialized_content.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        yaml_serialize_to_string(ft_nullptr, ft_nullptr, serialized_content));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        yaml_deserialize_from_string(serialized_content, ft_nullptr,
            ft_nullptr));
    return (1);
}

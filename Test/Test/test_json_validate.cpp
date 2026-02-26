#include "../test_internal.hpp"
#include "../../JSon/json.hpp"
#include "../../JSon/document.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_json_validate_schema_success, "json schema validation success")
{
    json_document document;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, json_group_list_enable_thread_safety());
    if (document.initialize() != FT_ERR_SUCCESS)
        return (0);
    json_group *group = document.create_group("user");
    json_item *item_name = document.create_item("name", "Alice");
    json_item *item_age = document.create_item("age", 30);
    document.add_item(group, item_name);
    document.add_item(group, item_age);
    document.append_group(group);
    json_schema_field name_field;
    name_field.key = "name";
    name_field.type = JSON_STRING;
    name_field.required = true;
    name_field.next = ft_nullptr;
    name_field._mutex = ft_nullptr;
    name_field._error_code = FT_ERR_SUCCESS;
    json_schema_field age_field;
    age_field.key = "age";
    age_field.type = JSON_NUMBER;
    age_field.required = true;
    age_field.next = ft_nullptr;
    age_field._mutex = ft_nullptr;
    age_field._error_code = FT_ERR_SUCCESS;
    name_field.next = &age_field;
    json_schema schema;
    schema.fields = &name_field;
    schema._mutex = ft_nullptr;
    schema._error_code = FT_ERR_SUCCESS;
    bool result = json_validate_schema(group, schema);
    FT_ASSERT_EQ(true, result);
    (void)json_group_list_disable_thread_safety();
    return (1);
}

FT_TEST(test_json_validate_schema_missing_field, "json schema validation missing field")
{
    json_document document;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, json_group_list_enable_thread_safety());
    if (document.initialize() != FT_ERR_SUCCESS)
        return (0);
    json_group *group = document.create_group("user");
    json_item *item_name = document.create_item("name", "Alice");
    document.add_item(group, item_name);
    document.append_group(group);
    json_schema_field name_field;
    name_field.key = "name";
    name_field.type = JSON_STRING;
    name_field.required = true;
    name_field.next = ft_nullptr;
    name_field._mutex = ft_nullptr;
    name_field._error_code = FT_ERR_SUCCESS;
    json_schema_field age_field;
    age_field.key = "age";
    age_field.type = JSON_NUMBER;
    age_field.required = true;
    age_field.next = ft_nullptr;
    age_field._mutex = ft_nullptr;
    age_field._error_code = FT_ERR_SUCCESS;
    name_field.next = &age_field;
    json_schema schema;
    schema.fields = &name_field;
    schema._mutex = ft_nullptr;
    schema._error_code = FT_ERR_SUCCESS;
    bool result = json_validate_schema(group, schema);
    FT_ASSERT_EQ(false, result);
    (void)json_group_list_disable_thread_safety();
    return (1);
}

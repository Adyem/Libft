#include "../JSon/json.hpp"
#include "../JSon/document.hpp"
#include "../CPP_class/class_nullptr.hpp"

int test_json_validate_success(void)
{
    json_document document;
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
    json_schema_field age_field;
    age_field.key = "age";
    age_field.type = JSON_NUMBER;
    age_field.required = true;
    age_field.next = ft_nullptr;
    name_field.next = &age_field;
    json_schema schema;
    schema.fields = &name_field;
    if (json_validate(group, schema) == true)
        return (1);
    return (0);
}

int test_json_validate_missing_field(void)
{
    json_document document;
    json_group *group = document.create_group("user");
    json_item *item_name = document.create_item("name", "Alice");
    document.add_item(group, item_name);
    document.append_group(group);
    json_schema_field name_field;
    name_field.key = "name";
    name_field.type = JSON_STRING;
    name_field.required = true;
    name_field.next = ft_nullptr;
    json_schema_field age_field;
    age_field.key = "age";
    age_field.type = JSON_NUMBER;
    age_field.required = true;
    age_field.next = ft_nullptr;
    name_field.next = &age_field;
    json_schema schema;
    schema.fields = &name_field;
    if (json_validate(group, schema) == false)
        return (1);
    return (0);
}

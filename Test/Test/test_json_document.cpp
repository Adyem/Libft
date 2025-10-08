#include "../../JSon/document.hpp"
#include "../../JSon/json.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#include <string>

FT_TEST(test_json_document_find_item_by_pointer_success, "json document resolves pointers to items")
{
    json_document document;
    json_group *group = document.create_group("config");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = document.create_item("name", "value");
    FT_ASSERT(item != ft_nullptr);
    document.append_group(group);
    document.add_item(group, item);
    json_item *found_item = document.find_item_by_pointer("/config/name");
    FT_ASSERT(found_item == item);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    const char *value = document.get_value_by_pointer("/config/name");
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(std::string("value"), std::string(value));
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    return (1);
}

FT_TEST(test_json_document_find_item_by_pointer_missing_path_sets_error, "json document reports missing pointer paths")
{
    json_document document;
    json_group *group = document.create_group("config");
    FT_ASSERT(group != ft_nullptr);
    document.append_group(group);
    json_item *missing = document.find_item_by_pointer("/missing/name");
    FT_ASSERT(missing == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, document.get_error());
    return (1);
}

FT_TEST(test_json_document_pointer_unescapes_tokens, "json document pointer decoding supports escaped segments")
{
    json_document document;
    json_group *group = document.create_group("config/advanced");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = document.create_item("name~value", "42");
    FT_ASSERT(item != ft_nullptr);
    document.append_group(group);
    document.add_item(group, item);
    json_item *found_item = document.find_item_by_pointer("/config~1advanced/name~0value");
    FT_ASSERT(found_item == item);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    const char *value = document.get_value_by_pointer("/config~1advanced/name~0value");
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(std::string("42"), std::string(value));
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    return (1);
}

FT_TEST(test_json_document_pointer_requires_leading_slash, "json document pointer requires a leading slash")
{
    json_document document;
    json_group *group = document.create_group("config");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = document.create_item("name", "value");
    FT_ASSERT(item != ft_nullptr);
    document.append_group(group);
    document.add_item(group, item);
    json_item *found_item = document.find_item_by_pointer("config/name");
    FT_ASSERT(found_item == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    return (1);
}

FT_TEST(test_json_document_pointer_rejects_empty_segment, "json document pointer rejects empty path segments")
{
    json_document document;
    json_group *group = document.create_group("config");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = document.create_item("name", "value");
    FT_ASSERT(item != ft_nullptr);
    document.append_group(group);
    document.add_item(group, item);
    json_item *found_item = document.find_item_by_pointer("/config//name");
    FT_ASSERT(found_item == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    return (1);
}

FT_TEST(test_json_document_pointer_invalid_escape_sets_error, "json document pointer invalid escapes set errors")
{
    json_document document;
    json_group *group = document.create_group("config");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = document.create_item("name", "value");
    FT_ASSERT(item != ft_nullptr);
    document.append_group(group);
    document.add_item(group, item);
    json_item *found_item = document.find_item_by_pointer("/config/~2name");
    FT_ASSERT(found_item == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    return (1);
}

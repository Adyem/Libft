#include "../../Parser/dom.hpp"
#include "../../JSon/document.hpp"
#include "../../JSon/json_dom_bridge.hpp"
#include "../../JSon/json.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../XML/xml.hpp"
#include "../../XML/xml_dom_bridge.hpp"
#include "../../YAML/yaml.hpp"
#include "../../YAML/yaml_dom_bridge.hpp"
#include "../../CMA/CMA.hpp"

#include <new>
#include <string>
#include <memory>

static void yaml_value_deleter(yaml_value *value)
{
    if (value != ft_nullptr)
        yaml_free(value);
    return ;
}

static void initialize_simple_dom_document(ft_dom_document &document)
{
    ft_dom_node *root_node;
    ft_dom_node *child_node;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
        return ;
    root_node->set_type(FT_DOM_NODE_OBJECT);
    root_node->set_name("root");
    child_node = new(std::nothrow) ft_dom_node();
    if (!child_node)
    {
        delete root_node;
        return ;
    }
    child_node->set_type(FT_DOM_NODE_ELEMENT);
    child_node->set_name("child");
    root_node->add_child(child_node);
    document.set_root(root_node);
    return ;
}

FT_TEST(test_dom_find_path_locates_nested_nodes, "ft_dom_find_path locates nested nodes")
{
    ft_dom_document document;
    ft_dom_node *root_node;
    const ft_dom_node *found_node;
    ft_string child_path("child");

    initialize_simple_dom_document(document);
    root_node = document.get_root();
    FT_ASSERT(root_node != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    found_node = ft_nullptr;
    FT_ASSERT_EQ(0, ft_dom_find_path(root_node, child_path, &found_node));
    FT_ASSERT(found_node != ft_nullptr);
    FT_ASSERT(std::string(found_node->get_name().c_str()) == "child");
    FT_ASSERT_EQ(ER_SUCCESS, root_node->get_error());
    return (1);
}

FT_TEST(test_dom_find_path_reports_missing_segments, "ft_dom_find_path reports missing segments")
{
    ft_dom_document document;
    ft_dom_node *root_node;
    const ft_dom_node *found_node;
    ft_string missing_path("missing");

    initialize_simple_dom_document(document);
    root_node = document.get_root();
    FT_ASSERT(root_node != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    found_node = ft_nullptr;
    FT_ASSERT_EQ(-1, ft_dom_find_path(root_node, missing_path, &found_node));
    FT_ASSERT(found_node == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_dom_schema_reports_missing_required_nodes, "ft_dom_schema marks reports for missing required nodes")
{
    ft_dom_document document;
    ft_dom_schema schema;
    ft_dom_validation_report report;

    initialize_simple_dom_document(document);
    FT_ASSERT_EQ(0, schema.add_rule("child", FT_DOM_NODE_ELEMENT, true));
    FT_ASSERT_EQ(0, schema.add_rule("missing", FT_DOM_NODE_ELEMENT, true));
    FT_ASSERT_EQ(0, schema.validate(document, report));
    FT_ASSERT_EQ(false, report.valid());
    const ft_vector<ft_dom_validation_error> &errors = report.errors();
    FT_ASSERT_EQ(static_cast<size_t>(1), errors.size());
    const ft_dom_validation_error &error_entry = errors[0];
    FT_ASSERT(std::string(error_entry.path.c_str()) == "missing");
    FT_ASSERT(std::string(error_entry.message.c_str()) == "Required node missing");
    return (1);
}

FT_TEST(test_dom_schema_reports_type_mismatches, "ft_dom_schema reports node type mismatches")
{
    ft_dom_document document;
    ft_dom_schema schema;
    ft_dom_validation_report report;
    ft_dom_node *root_node;
    const ft_vector<ft_dom_node*> *children_pointer;
    ft_dom_node *child_node;

    initialize_simple_dom_document(document);
    root_node = document.get_root();
    FT_ASSERT(root_node != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    children_pointer = &root_node->get_children();
    FT_ASSERT_EQ(ER_SUCCESS, root_node->get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), children_pointer->size());
    FT_ASSERT_EQ(ER_SUCCESS, children_pointer->get_error());
    child_node = (*children_pointer)[0];
    FT_ASSERT(child_node != ft_nullptr);
    child_node->set_type(FT_DOM_NODE_VALUE);
    FT_ASSERT_EQ(ER_SUCCESS, child_node->get_error());
    FT_ASSERT_EQ(0, schema.add_rule("child", FT_DOM_NODE_OBJECT, true));
    FT_ASSERT_EQ(0, schema.validate(document, report));
    FT_ASSERT_EQ(false, report.valid());
    const ft_vector<ft_dom_validation_error> &errors = report.errors();
    FT_ASSERT_EQ(static_cast<size_t>(1), errors.size());
    const ft_dom_validation_error &error_entry = errors[0];
    FT_ASSERT(std::string(error_entry.path.c_str()) == "child");
    FT_ASSERT(std::string(error_entry.message.c_str()) == "Node type mismatch");
    return (1);
}

FT_TEST(test_json_dom_bridge_round_trip, "json dom bridge round trips document data")
{
    json_document source_document;
    json_group *settings_group;
    json_item *name_item;
    json_item *enabled_item;
    json_item *big_number_item;
    ft_big_number big_number_value;

    settings_group = source_document.create_group("settings");
    FT_ASSERT(settings_group != ft_nullptr);
    name_item = source_document.create_item("name", "example");
    FT_ASSERT(name_item != ft_nullptr);
    enabled_item = source_document.create_item("enabled", "true");
    FT_ASSERT(enabled_item != ft_nullptr);
    big_number_value.assign("12345678901234567890");
    FT_ASSERT_EQ(ER_SUCCESS, big_number_value.get_error());
    big_number_item = source_document.create_item("count", big_number_value);
    FT_ASSERT(big_number_item != ft_nullptr);
    source_document.append_group(settings_group);
    source_document.add_item(settings_group, name_item);
    source_document.add_item(settings_group, enabled_item);
    source_document.add_item(settings_group, big_number_item);
    FT_ASSERT_EQ(ER_SUCCESS, source_document.get_error());

    ft_dom_document dom_document;
    FT_ASSERT_EQ(0, json_document_to_dom(source_document, dom_document));
    ft_dom_node *dom_root = dom_document.get_root();
    FT_ASSERT(dom_root != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, dom_document.get_error());
    FT_ASSERT(std::string(dom_root->get_name().c_str()) == "json");
    const ft_vector<ft_dom_node*> &group_nodes = dom_root->get_children();
    FT_ASSERT_EQ(static_cast<size_t>(1), group_nodes.size());
    ft_dom_node *group_node = group_nodes[0];
    FT_ASSERT(group_node != ft_nullptr);
    FT_ASSERT(std::string(group_node->get_name().c_str()) == "settings");
    const ft_vector<ft_dom_node*> &item_nodes = group_node->get_children();
    FT_ASSERT_EQ(static_cast<size_t>(3), item_nodes.size());

    size_t index;
    size_t count;

    index = 0;
    count = item_nodes.size();
    bool found_big_number;

    found_big_number = false;
    while (index < count)
    {
        ft_dom_node *item_node = item_nodes[index];
        FT_ASSERT(item_node != ft_nullptr);
        if (std::string(item_node->get_name().c_str()) == "count")
        {
            ft_string attribute_key("json:type");
            ft_string attribute_value = item_node->get_attribute(attribute_key);
            FT_ASSERT_EQ(ER_SUCCESS, item_node->get_error());
            FT_ASSERT(std::string(attribute_value.c_str()) == "big_number");
            found_big_number = true;
        }
        index += 1;
    }
    FT_ASSERT(found_big_number);

    json_document round_trip_document;
    FT_ASSERT_EQ(0, json_document_from_dom(dom_document, round_trip_document));
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_document.get_error());
    json_group *round_trip_group = round_trip_document.find_group("settings");
    FT_ASSERT(round_trip_group != ft_nullptr);
    json_item *round_trip_name = round_trip_document.find_item(round_trip_group, "name");
    FT_ASSERT(round_trip_name != ft_nullptr);
    FT_ASSERT(std::string(round_trip_name->value) == "example");
    json_item *round_trip_count = round_trip_document.find_item(round_trip_group, "count");
    FT_ASSERT(round_trip_count != ft_nullptr);
    FT_ASSERT(round_trip_count->is_big_number);
    FT_ASSERT(round_trip_count->big_number != ft_nullptr);
    FT_ASSERT(std::string(round_trip_count->big_number->to_string_base(10).c_str()) == "12345678901234567890");
    return (1);
}

FT_TEST(test_json_dom_bridge_rejects_non_object_root, "json dom bridge rejects non object root nodes")
{
    ft_dom_document dom_document;
    ft_dom_node *root_node;
    json_document document;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
        return (0);
    root_node->set_type(FT_DOM_NODE_VALUE);
    if (root_node->get_error() != ER_SUCCESS)
    {
        delete root_node;
        return (0);
    }
    if (root_node->set_name("root") != 0)
    {
        delete root_node;
        return (0);
    }
    if (root_node->set_value("content") != 0)
    {
        delete root_node;
        return (0);
    }
    dom_document.set_root(root_node);
    FT_ASSERT_EQ(ER_SUCCESS, dom_document.get_error());
    FT_ASSERT_EQ(-1, json_document_from_dom(dom_document, document));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    return (1);
}

FT_TEST(test_xml_dom_bridge_round_trip, "xml dom bridge round trips document data")
{
    xml_document source_document;

    FT_ASSERT_EQ(ER_SUCCESS, source_document.load_from_string("<root attr=\"value\"><child>text</child></root>"));
    ft_dom_document dom_document;
    FT_ASSERT_EQ(0, xml_document_to_dom(source_document, dom_document));
    ft_dom_node *dom_root = dom_document.get_root();
    FT_ASSERT(dom_root != ft_nullptr);
    FT_ASSERT(std::string(dom_root->get_name().c_str()) == "root");
    ft_string attribute_key("attr");
    ft_string attribute_value = dom_root->get_attribute(attribute_key);
    FT_ASSERT_EQ(ER_SUCCESS, dom_root->get_error());
    FT_ASSERT(std::string(attribute_value.c_str()) == "value");
    const ft_vector<ft_dom_node*> &child_nodes = dom_root->get_children();
    FT_ASSERT_EQ(static_cast<size_t>(1), child_nodes.size());
    ft_dom_node *child_node = child_nodes[0];
    FT_ASSERT(child_node != ft_nullptr);
    FT_ASSERT(std::string(child_node->get_name().c_str()) == "child");
    FT_ASSERT(std::string(child_node->get_value().c_str()) == "text");

    xml_document round_trip_document;
    FT_ASSERT_EQ(0, xml_document_from_dom(dom_document, round_trip_document));
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_document.get_error());
    char *serialized = round_trip_document.write_to_string();
    FT_ASSERT(serialized != ft_nullptr);
    bool matches_expected;

    matches_expected = std::string(serialized).find("<child>text</child>") != std::string::npos;
    cma_free(serialized);
    FT_ASSERT(matches_expected);
    return (1);
}

FT_TEST(test_yaml_dom_bridge_round_trip, "yaml dom bridge round trips maps and arrays")
{
    std::unique_ptr<yaml_value, void (*)(yaml_value *)> root_guard(ft_nullptr, &yaml_value_deleter);
    std::unique_ptr<yaml_value> items_guard;
    std::unique_ptr<yaml_value> first_value_guard;
    std::unique_ptr<yaml_value> second_value_guard;
    yaml_value *root_pointer;
    yaml_value *items_pointer;
    ft_dom_document dom_document;
    ft_dom_node *dom_root;
    ft_dom_node *items_node;
    ft_dom_node *first_item_node;
    std::unique_ptr<yaml_value, void (*)(yaml_value *)> round_trip_guard(ft_nullptr, &yaml_value_deleter);
    yaml_value *round_trip_pointer;
    yaml_value *round_trip_items;
    yaml_value *round_trip_first_value;
    yaml_value *round_trip_second_value;
    ft_string items_key("items");
    FT_ASSERT_EQ(ER_SUCCESS, items_key.get_error());

    root_guard.reset(new (std::nothrow) yaml_value());
    if (!root_guard.get())
        return (0);
    root_pointer = root_guard.get();
    root_pointer->set_type(YAML_MAP);
    if (root_pointer->get_error() != ER_SUCCESS)
        return (0);
    items_guard.reset(new (std::nothrow) yaml_value());
    if (!items_guard.get())
        return (0);
    items_pointer = items_guard.get();
    items_pointer->set_type(YAML_LIST);
    if (items_pointer->get_error() != ER_SUCCESS)
        return (0);
    first_value_guard.reset(new (std::nothrow) yaml_value());
    if (!first_value_guard.get())
        return (0);
    first_value_guard->set_scalar("one");
    if (first_value_guard->get_error() != ER_SUCCESS)
        return (0);
    items_pointer->add_list_item(first_value_guard.get());
    if (items_pointer->get_error() != ER_SUCCESS)
        return (0);
    first_value_guard.release();
    second_value_guard.reset(new (std::nothrow) yaml_value());
    if (!second_value_guard.get())
        return (0);
    second_value_guard->set_scalar("two");
    if (second_value_guard->get_error() != ER_SUCCESS)
        return (0);
    items_pointer->add_list_item(second_value_guard.get());
    if (items_pointer->get_error() != ER_SUCCESS)
        return (0);
    second_value_guard.release();
    root_pointer->add_map_item("items", items_pointer);
    if (root_pointer->get_error() != ER_SUCCESS)
        return (0);
    items_guard.release();
    FT_ASSERT_EQ(0, yaml_value_to_dom(root_pointer, dom_document));
    dom_root = dom_document.get_root();
    FT_ASSERT(dom_root != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, dom_document.get_error());
    FT_ASSERT_EQ(FT_DOM_NODE_OBJECT, dom_root->get_type());
    FT_ASSERT_EQ(ER_SUCCESS, dom_root->get_error());
    const ft_vector<ft_dom_node*> &root_children = dom_root->get_children();
    FT_ASSERT_EQ(static_cast<size_t>(1), root_children.size());
    FT_ASSERT_EQ(ER_SUCCESS, root_children.get_error());
    items_node = root_children[0];
    FT_ASSERT(items_node != ft_nullptr);
    FT_ASSERT(std::string(items_node->get_name().c_str()) == "items");
    FT_ASSERT_EQ(FT_DOM_NODE_ARRAY, items_node->get_type());
    FT_ASSERT_EQ(ER_SUCCESS, items_node->get_error());
    const ft_vector<ft_dom_node*> &item_children = items_node->get_children();
    FT_ASSERT_EQ(static_cast<size_t>(2), item_children.size());
    FT_ASSERT_EQ(ER_SUCCESS, item_children.get_error());
    first_item_node = item_children[0];
    FT_ASSERT(first_item_node != ft_nullptr);
    FT_ASSERT(std::string(first_item_node->get_value().c_str()) == "one");
    FT_ASSERT_EQ(ER_SUCCESS, first_item_node->get_error());
    round_trip_guard.reset(yaml_value_from_dom(dom_document));
    FT_ASSERT(round_trip_guard.get() != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    round_trip_pointer = round_trip_guard.get();
    FT_ASSERT_EQ(YAML_MAP, round_trip_pointer->get_type());
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_pointer->get_error());
    const ft_map<ft_string, yaml_value*> &round_trip_map = round_trip_pointer->get_map();
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_pointer->get_error());
    round_trip_items = round_trip_map.at(items_key);
    FT_ASSERT(round_trip_items != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_map.get_error());
    FT_ASSERT_EQ(YAML_LIST, round_trip_items->get_type());
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_items->get_error());
    const ft_vector<yaml_value*> &round_trip_list = round_trip_items->get_list();
    FT_ASSERT_EQ(static_cast<size_t>(2), round_trip_list.size());
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_list.get_error());
    round_trip_first_value = round_trip_list[0];
    FT_ASSERT(round_trip_first_value != ft_nullptr);
    FT_ASSERT(std::string(round_trip_first_value->get_scalar().c_str()) == "one");
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_first_value->get_error());
    round_trip_second_value = round_trip_list[1];
    FT_ASSERT(round_trip_second_value != ft_nullptr);
    FT_ASSERT(std::string(round_trip_second_value->get_scalar().c_str()) == "two");
    FT_ASSERT_EQ(ER_SUCCESS, round_trip_second_value->get_error());
    return (1);
}

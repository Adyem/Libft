#include "json_dom_bridge.hpp"
#include "json.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <new>

static void json_dom_delete_node(ft_dom_node *node) noexcept
{
    if (!node)
        return ;
    delete node;
    return ;
}

static int32_t json_dom_append_item(const json_item *item, ft_dom_node *group_node) noexcept
{
    int32_t node_initialize_error;

    if (!item || !group_node)
    {
        return (FT_ERR_INVALID_STATE);
    }
    ft_dom_node *item_node;

    item_node = new(std::nothrow) ft_dom_node();
    if (!item_node)
    {
        return (FT_ERR_INVALID_STATE);
    }
    node_initialize_error = item_node->initialize();
    if (node_initialize_error != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(item_node);
        return (node_initialize_error);
    }
    item_node->set_type(FT_DOM_NODE_VALUE);
    const char *item_key;

    item_key = item->key;
    if (!item_key)
        item_key = "";
    if (item_node->set_name(item_key) != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(item_node);
        return (FT_ERR_INVALID_STATE);
    }
    if (item->is_big_number)
    {
        if (!item->big_number)
        {
            json_dom_delete_node(item_node);
            return (FT_ERR_INVALID_STATE);
        }
        ft_string number_string;

        number_string = item->big_number->to_string_base(10);
        if (number_string.get_error() != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(item_node);
            return (FT_ERR_INVALID_STATE);
        }
        if (item_node->set_value(number_string) != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(item_node);
            return (FT_ERR_INVALID_STATE);
        }
        if (item_node->add_attribute("json:type", "big_number") != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(item_node);
            return (FT_ERR_INVALID_STATE);
        }
    }
    else
    {
        const char *item_value;

        item_value = item->value;
        if (!item_value)
            item_value = "";
        if (item_node->set_value(item_value) != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(item_node);
            return (FT_ERR_INVALID_STATE);
        }
    }
    if (group_node->add_child(item_node) != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(item_node);
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t json_dom_append_group(const json_group *group, ft_dom_node *root_node) noexcept
{
    int32_t node_initialize_error;

    if (!group || !root_node)
    {
        return (FT_ERR_INVALID_STATE);
    }
    ft_dom_node *group_node;

    group_node = new(std::nothrow) ft_dom_node();
    if (!group_node)
    {
        return (FT_ERR_INVALID_STATE);
    }
    node_initialize_error = group_node->initialize();
    if (node_initialize_error != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(group_node);
        return (node_initialize_error);
    }
    group_node->set_type(FT_DOM_NODE_OBJECT);
    const char *group_name;

    group_name = group->name;
    if (!group_name)
        group_name = "";
    if (group_node->set_name(group_name) != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(group_node);
        return (FT_ERR_INVALID_STATE);
    }
    const json_item *item_iterator;

    item_iterator = group->items;
    while (item_iterator)
    {
        if (json_dom_append_item(item_iterator, group_node) != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(group_node);
            return (FT_ERR_INVALID_STATE);
        }
        item_iterator = item_iterator->next;
    }
    if (root_node->add_child(group_node) != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(group_node);
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESS);
}

int32_t json_document_to_dom(const json_document &document, ft_dom_document &dom) noexcept
{
    int32_t root_initialize_error;

    dom.clear();
    ft_dom_node *root_node;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
    {
        return (FT_ERR_INVALID_STATE);
    }
    root_initialize_error = root_node->initialize();
    if (root_initialize_error != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(root_node);
        return (root_initialize_error);
    }
    root_node->set_type(FT_DOM_NODE_OBJECT);
    if (root_node->set_name("json" ) != FT_ERR_SUCCESS)
    {
        json_dom_delete_node(root_node);
        return (FT_ERR_INVALID_STATE);
    }
    json_group *group_iterator;

    group_iterator = document.get_groups();
    while (group_iterator)
    {
        if (json_dom_append_group(group_iterator, root_node) != FT_ERR_SUCCESS)
        {
            json_dom_delete_node(root_node);
            return (FT_ERR_INVALID_STATE);
        }
        group_iterator = group_iterator->next;
    }
    dom.set_root(root_node);
    return (FT_ERR_SUCCESS);
}

static int32_t json_dom_apply_item(ft_dom_node *item_node, json_group *group, json_document &document) noexcept
{
    if (!item_node || !group)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    const ft_string &item_name = item_node->get_name();
    const char *item_key;

    item_key = item_name.c_str();
    if (!item_key)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    const ft_string &item_value_string = item_node->get_value();
    const char *item_value;

    item_value = item_value_string.c_str();
    if (!item_value)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    ft_string type_key;
    int32_t type_initialize_error = type_key.initialize("json:type");
    if (type_initialize_error != FT_ERR_SUCCESS)
    {
        document.set_manual_error(type_initialize_error);
        return (FT_ERR_INVALID_STATE);
    }
    ft_string type_attribute = item_node->get_attribute(type_key);
    ft_bool has_type_attribute;

    has_type_attribute = (type_attribute.size() > 0);
    if (has_type_attribute && type_attribute == "big_number")
    {
        ft_big_number big_number_value;
        int32_t big_number_error;
        int32_t big_number_initialize_error;

        big_number_initialize_error = big_number_value.initialize();
        if (big_number_initialize_error != FT_ERR_SUCCESS)
        {
            document.set_manual_error(big_number_initialize_error);
            return (FT_ERR_INVALID_STATE);
        }
        big_number_value.assign(item_value);
        big_number_error = big_number_value.get_error();
        if (big_number_error != FT_ERR_SUCCESS)
        {
            document.set_manual_error(big_number_error);
            return (FT_ERR_INVALID_STATE);
        }
        json_item *json_item_pointer;

        json_item_pointer = document.create_item(item_key, big_number_value);
        if (!json_item_pointer)
            return (FT_ERR_INVALID_STATE);
        document.add_item(group, json_item_pointer);
        return (FT_ERR_SUCCESS);
    }
    json_item *json_item_pointer;

    json_item_pointer = document.create_item(item_key, item_value);
    if (!json_item_pointer)
        return (FT_ERR_INVALID_STATE);
    document.add_item(group, json_item_pointer);
    return (FT_ERR_SUCCESS);
}

static int32_t json_dom_apply_group(ft_dom_node *group_node, json_document &document) noexcept
{
    if (!group_node)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    const ft_string &group_name = group_node->get_name();
    const char *group_name_cstr;

    group_name_cstr = group_name.c_str();
    if (!group_name_cstr)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    json_group *group_pointer;

    group_pointer = document.create_group(group_name_cstr);
    if (!group_pointer)
        return (FT_ERR_INVALID_STATE);
    document.append_group(group_pointer);
    const ft_vector<ft_dom_node*> &children = group_node->get_children();
    ft_size_t index;
    ft_size_t count;

    index = 0;
    count = children.size();
    while (index < count)
    {
        ft_dom_node *child_node = children[index];
        if (json_dom_apply_item(child_node, group_pointer, document) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int32_t json_document_from_dom(const ft_dom_document &dom, json_document &document) noexcept
{
    document.clear();
    ft_dom_node *root_node;
    ft_dom_node_type root_type;

    root_node = dom.get_root();
    if (!root_node)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }

    root_type = root_node->get_type();
    if (root_type != FT_DOM_NODE_OBJECT)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_STATE);
    }
    const ft_vector<ft_dom_node*> &groups = root_node->get_children();

    ft_size_t index;
    ft_size_t count;

    index = 0;
    count = groups.size();
    while (index < count)
    {
        ft_dom_node *group_node = groups[index];
        if (json_dom_apply_group(group_node, document) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
        index += 1;
    }
    document.set_manual_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

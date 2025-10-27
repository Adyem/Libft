#include "json_dom_bridge.hpp"
#include "json.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <new>

static void json_dom_delete_node(ft_dom_node *node) noexcept
{
    if (!node)
        return ;
    delete node;
    return ;
}

static int json_dom_append_item(const json_item *item, ft_dom_node *group_node) noexcept
{
    if (!item || !group_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_dom_node *item_node;

    item_node = new(std::nothrow) ft_dom_node();
    if (!item_node)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    item_node->set_type(FT_DOM_NODE_VALUE);
    if (item_node->get_error() != ER_SUCCESS)
    {
        json_dom_delete_node(item_node);
        return (-1);
    }
    const char *item_key;

    item_key = item->key;
    if (!item_key)
        item_key = "";
    if (item_node->set_name(item_key) != 0)
    {
        json_dom_delete_node(item_node);
        return (-1);
    }
    if (item->is_big_number)
    {
        if (!item->big_number)
        {
            json_dom_delete_node(item_node);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        ft_string number_string;

        number_string = item->big_number->to_string_base(10);
        if (number_string.get_error() != ER_SUCCESS)
        {
            json_dom_delete_node(item_node);
            ft_errno = number_string.get_error();
            return (-1);
        }
        if (item_node->set_value(number_string) != 0)
        {
            json_dom_delete_node(item_node);
            return (-1);
        }
        if (item_node->add_attribute("json:type", "big_number") != 0)
        {
            json_dom_delete_node(item_node);
            return (-1);
        }
    }
    else
    {
        const char *item_value;

        item_value = item->value;
        if (!item_value)
            item_value = "";
        if (item_node->set_value(item_value) != 0)
        {
            json_dom_delete_node(item_node);
            return (-1);
        }
    }
    if (group_node->add_child(item_node) != 0)
    {
        json_dom_delete_node(item_node);
        return (-1);
    }
    return (0);
}

static int json_dom_append_group(const json_group *group, ft_dom_node *root_node) noexcept
{
    if (!group || !root_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_dom_node *group_node;

    group_node = new(std::nothrow) ft_dom_node();
    if (!group_node)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    group_node->set_type(FT_DOM_NODE_OBJECT);
    if (group_node->get_error() != ER_SUCCESS)
    {
        json_dom_delete_node(group_node);
        return (-1);
    }
    const char *group_name;

    group_name = group->name;
    if (!group_name)
        group_name = "";
    if (group_node->set_name(group_name) != 0)
    {
        json_dom_delete_node(group_node);
        return (-1);
    }
    const json_item *item_iterator;

    item_iterator = group->items;
    while (item_iterator)
    {
        if (json_dom_append_item(item_iterator, group_node) != 0)
        {
            json_dom_delete_node(group_node);
            return (-1);
        }
        item_iterator = item_iterator->next;
    }
    if (root_node->add_child(group_node) != 0)
    {
        json_dom_delete_node(group_node);
        return (-1);
    }
    return (0);
}

int json_document_to_dom(const json_document &document, ft_dom_document &dom) noexcept
{
    dom.clear();
    if (dom.get_error() != ER_SUCCESS)
        return (-1);
    ft_dom_node *root_node;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    root_node->set_type(FT_DOM_NODE_OBJECT);
    if (root_node->get_error() != ER_SUCCESS)
    {
        json_dom_delete_node(root_node);
        return (-1);
    }
    if (root_node->set_name("json" ) != 0)
    {
        json_dom_delete_node(root_node);
        return (-1);
    }
    json_group *group_iterator;

    group_iterator = document.get_groups();
    if (document.get_error() != ER_SUCCESS)
    {
        json_dom_delete_node(root_node);
        return (-1);
    }
    while (group_iterator)
    {
        if (json_dom_append_group(group_iterator, root_node) != 0)
        {
            json_dom_delete_node(root_node);
            return (-1);
        }
        group_iterator = group_iterator->next;
    }
    dom.set_root(root_node);
    if (dom.get_error() != ER_SUCCESS)
    {
        json_dom_delete_node(root_node);
        return (-1);
    }
    return (0);
}

static int json_dom_apply_item(ft_dom_node *item_node, json_group *group, json_document &document) noexcept
{
    if (!item_node || !group)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    const ft_string &item_name = item_node->get_name();

    if (item_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(item_node->get_error());
        return (-1);
    }
    const char *item_key;

    item_key = item_name.c_str();
    if (!item_key)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    const ft_string &item_value_string = item_node->get_value();

    if (item_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(item_node->get_error());
        return (-1);
    }
    const char *item_value;

    item_value = item_value_string.c_str();
    if (!item_value)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_string type_attribute = item_node->get_attribute("json:type");
    bool has_type_attribute;

    has_type_attribute = (item_node->get_error() == ER_SUCCESS);
    if (has_type_attribute && type_attribute == "big_number")
    {
        ft_big_number big_number_value;

        big_number_value.assign(item_value);
        if (big_number_value.get_error() != ER_SUCCESS)
        {
            document.set_manual_error(big_number_value.get_error());
            return (-1);
        }
        json_item *json_item_pointer;

        json_item_pointer = document.create_item(item_key, big_number_value);
        if (!json_item_pointer)
            return (-1);
        document.add_item(group, json_item_pointer);
        if (document.get_error() != ER_SUCCESS)
            return (-1);
        return (0);
    }
    json_item *json_item_pointer;

    json_item_pointer = document.create_item(item_key, item_value);
    if (!json_item_pointer)
        return (-1);
    document.add_item(group, json_item_pointer);
    if (document.get_error() != ER_SUCCESS)
        return (-1);
    return (0);
}

static int json_dom_apply_group(ft_dom_node *group_node, json_document &document) noexcept
{
    if (!group_node)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    const ft_string &group_name = group_node->get_name();

    if (group_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(group_node->get_error());
        return (-1);
    }
    const char *group_name_cstr;

    group_name_cstr = group_name.c_str();
    if (!group_name_cstr)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_group *group_pointer;

    group_pointer = document.create_group(group_name_cstr);
    if (!group_pointer)
        return (-1);
    document.append_group(group_pointer);
    if (document.get_error() != ER_SUCCESS)
        return (-1);
    const ft_vector<ft_dom_node*> &children = group_node->get_children();

    if (group_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(group_node->get_error());
        return (-1);
    }
    size_t index;
    size_t count;

    index = 0;
    count = children.size();
    while (index < count)
    {
        ft_dom_node *child_node = children[index];

        if (children.get_error() != ER_SUCCESS)
        {
            document.set_manual_error(children.get_error());
            return (-1);
        }
        if (json_dom_apply_item(child_node, group_pointer, document) != 0)
            return (-1);
        index += 1;
    }
    return (0);
}

int json_document_from_dom(const ft_dom_document &dom, json_document &document) noexcept
{
    document.clear();
    if (document.get_error() != ER_SUCCESS)
        return (-1);
    ft_dom_node *root_node;

    root_node = dom.get_root();
    if (dom.get_error() != ER_SUCCESS)
    {
        document.set_manual_error(dom.get_error());
        return (-1);
    }
    if (!root_node)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_dom_node_type root_type;

    root_type = root_node->get_type();
    if (root_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(root_node->get_error());
        return (-1);
    }
    if (root_type != FT_DOM_NODE_OBJECT)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    const ft_vector<ft_dom_node*> &groups = root_node->get_children();

    if (root_node->get_error() != ER_SUCCESS)
    {
        document.set_manual_error(root_node->get_error());
        return (-1);
    }
    size_t index;
    size_t count;

    index = 0;
    count = groups.size();
    while (index < count)
    {
        ft_dom_node *group_node = groups[index];

        if (groups.get_error() != ER_SUCCESS)
        {
        document.set_manual_error(groups.get_error());
            return (-1);
        }
        if (json_dom_apply_group(group_node, document) != 0)
            return (-1);
        index += 1;
    }
    document.set_manual_error(ER_SUCCESS);
    return (0);
}


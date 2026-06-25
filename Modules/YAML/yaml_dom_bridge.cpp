#include "yaml_dom_bridge.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

static void yaml_dom_delete_node(ft_dom_node *node) noexcept
{
    if (!node)
        return ;
    delete node;
    return ;
}

static int32_t yaml_dom_populate_node(const yaml_value *value, ft_dom_node *node) noexcept
{
    if (!value || !node)
        return (FT_ERR_INVALID_ARGUMENT);
    yaml_type type;

    type = value->get_type();
    if (type == YAML_SCALAR)
    {
        const ft_string &scalar = value->get_scalar();
        node->set_type(FT_DOM_NODE_VALUE);
        if (node->set_value(scalar) != 0)
            return (FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_SUCCESS);
    }
    if (type == YAML_LIST)
    {
        node->set_type(FT_DOM_NODE_ARRAY);
        const ft_vector<yaml_value*> &list = value->get_list();
        ft_size_t index;
        ft_size_t count;

        index = 0;
        count = list.size();
        while (index < count)
        {
            yaml_value *child_value;

            child_value = list[index];
            ft_dom_node *child_node;
            int32_t child_initialize_error;

            child_node = new(std::nothrow) ft_dom_node();
            if (!child_node)
                return (FT_ERR_NO_MEMORY);
            child_initialize_error = child_node->initialize();
            if (child_initialize_error != FT_ERR_SUCCESS)
            {
                yaml_dom_delete_node(child_node);
                return (child_initialize_error);
            }
            if (child_node->set_name("item") != 0)
            {
                yaml_dom_delete_node(child_node);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            int32_t populate_error;

            populate_error = yaml_dom_populate_node(child_value, child_node);
            if (populate_error != FT_ERR_SUCCESS)
            {
                yaml_dom_delete_node(child_node);
                return (populate_error);
            }
            if (node->add_child(child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (FT_ERR_INVALID_STATE);
            }
            index += 1;
        }
        return (FT_ERR_SUCCESS);
    }
    if (type == YAML_MAP)
    {
        node->set_type(FT_DOM_NODE_OBJECT);
        const ft_vector<ft_string> &keys = value->get_map_keys();
        const ft_map<ft_string, yaml_value*> &map_reference = value->get_map();
        ft_size_t key_index;
        ft_size_t key_count;

        key_index = 0;
        key_count = keys.size();
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];
            const char *key_cstr;

            key_cstr = key.c_str();
            if (!key_cstr)
                return (FT_ERR_INVALID_ARGUMENT);
            yaml_value *child_value;

            child_value = map_reference.at(key);
            ft_dom_node *child_node;
            int32_t child_initialize_error;

            child_node = new(std::nothrow) ft_dom_node();
            if (!child_node)
                return (FT_ERR_NO_MEMORY);
            child_initialize_error = child_node->initialize();
            if (child_initialize_error != FT_ERR_SUCCESS)
            {
                yaml_dom_delete_node(child_node);
                return (child_initialize_error);
            }
            if (child_node->set_name(key_cstr) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            int32_t populate_error;

            populate_error = yaml_dom_populate_node(child_value, child_node);
            if (populate_error != FT_ERR_SUCCESS)
            {
                yaml_dom_delete_node(child_node);
                return (populate_error);
            }
            if (node->add_child(child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (FT_ERR_INVALID_STATE);
            }
            key_index += 1;
        }
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_INVALID_ARGUMENT);
}

int32_t yaml_value_to_dom(const yaml_value *value, ft_dom_document &dom) noexcept
{
    if (!value)
        return (FT_ERR_INVALID_ARGUMENT);
    dom.clear();
    ft_dom_node *root_node;
    int32_t root_initialize_error;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
        return (FT_ERR_NO_MEMORY);
    root_initialize_error = root_node->initialize();
    if (root_initialize_error != FT_ERR_SUCCESS)
    {
        yaml_dom_delete_node(root_node);
        return (root_initialize_error);
    }
    if (root_node->set_name("yaml") != 0)
    {
        yaml_dom_delete_node(root_node);
        return (FT_ERR_INVALID_STATE);
    }
    int32_t populate_error;

    populate_error = yaml_dom_populate_node(value, root_node);
    if (populate_error != FT_ERR_SUCCESS)
    {
        yaml_dom_delete_node(root_node);
        return (populate_error);
    }
    dom.set_root(root_node);
    return (FT_ERR_SUCCESS);
}

static yaml_value *yaml_dom_build_value(ft_dom_node *node, int32_t *status) noexcept
{
    if (status)
        *status = FT_ERR_SUCCESS;
    if (!node)
    {
        if (status)
            *status = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_dom_node_type node_type;

    node_type = node->get_type();
    yaml_value *result;

    result = new(std::nothrow) yaml_value();
    if (!result)
    {
        if (status)
            *status = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (result->initialize() != FT_ERR_SUCCESS)
    {
        if (status)
            *status = FT_ERR_NO_MEMORY;
        delete result;
        return (ft_nullptr);
    }
    if (node_type == FT_DOM_NODE_VALUE)
    {
        const ft_string &scalar = node->get_value();
        result->set_type(YAML_SCALAR);
        result->set_scalar(scalar);
        return (result);
    }
    if (node_type == FT_DOM_NODE_ARRAY)
    {
        const ft_vector<ft_dom_node*> &children = node->get_children();
        ft_size_t index;
        ft_size_t count;

        index = 0;
        count = children.size();
        while (index < count)
        {
            ft_dom_node *child_node = children[index];
            int32_t child_status;
            yaml_value *child_value;

            child_value = yaml_dom_build_value(child_node, &child_status);
            if (!child_value)
            {
                if (status)
                    *status = child_status;
                delete result;
                return (ft_nullptr);
            }
            result->add_list_item(child_value);
            index += 1;
        }
        return (result);
    }
    if (node_type == FT_DOM_NODE_OBJECT)
    {
        const ft_vector<ft_dom_node*> &children = node->get_children();
        ft_size_t index;
        ft_size_t count;

        index = 0;
        count = children.size();
        while (index < count)
        {
            ft_dom_node *child_node = children[index];
            const ft_string &child_name = child_node->get_name();

            int32_t child_status;
            yaml_value *child_value;

            child_value = yaml_dom_build_value(child_node, &child_status);
            if (!child_value)
            {
                if (status)
                    *status = child_status;
                delete result;
                return (ft_nullptr);
            }
            result->add_map_item(child_name, child_value);
            index += 1;
        }
        return (result);
    }
    if (status)
        *status = FT_ERR_INVALID_ARGUMENT;
    delete result;
    return (ft_nullptr);
}

yaml_value *yaml_value_from_dom(const ft_dom_document &dom) noexcept
{
    ft_dom_node *root_node;

    root_node = dom.get_root();
    if (!root_node)
        return (ft_nullptr);
    int32_t status;
    yaml_value *result;

    result = yaml_dom_build_value(root_node, &status);
    if (!result)
    {
        if (status == FT_ERR_SUCCESS)
            status = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    return (result);
}

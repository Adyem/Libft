#include "yaml_dom_bridge.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <new>

static void yaml_dom_delete_node(ft_dom_node *node) noexcept
{
    if (!node)
        return ;
    delete node;
    return ;
}

static int yaml_dom_populate_node(const yaml_value *value, ft_dom_node *node) noexcept
{
    if (!value || !node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    yaml_type type;

    type = value->get_type();
    if (value->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = value->get_error();
        return (-1);
    }
    if (type == YAML_SCALAR)
    {
        const ft_string &scalar = value->get_scalar();

        if (value->get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = value->get_error();
            return (-1);
        }
        node->set_type(FT_DOM_NODE_VALUE);
        if (node->get_error() != FT_ERR_SUCCESSS)
            return (-1);
        if (node->set_value(scalar) != 0)
            return (-1);
        return (0);
    }
    if (type == YAML_LIST)
    {
        node->set_type(FT_DOM_NODE_ARRAY);
        if (node->get_error() != FT_ERR_SUCCESSS)
            return (-1);
        const ft_vector<yaml_value*> &list = value->get_list();

        if (value->get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = value->get_error();
            return (-1);
        }
        size_t index;
        size_t count;

        index = 0;
        count = list.size();
        while (index < count)
        {
            yaml_value *child_value;

            child_value = list[index];
            if (list.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = list.get_error();
                return (-1);
            }
            ft_dom_node *child_node;

            child_node = new(std::nothrow) ft_dom_node();
            if (!child_node)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                return (-1);
            }
            ft_string index_name;

            index_name = ft_to_string(static_cast<long>(index));
            if (index_name.get_error() != FT_ERR_SUCCESSS)
            {
                yaml_dom_delete_node(child_node);
                ft_errno = index_name.get_error();
                return (-1);
            }
            if (child_node->set_name(index_name) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            if (yaml_dom_populate_node(child_value, child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            if (node->add_child(child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            index += 1;
        }
        return (0);
    }
    if (type == YAML_MAP)
    {
        node->set_type(FT_DOM_NODE_OBJECT);
        if (node->get_error() != FT_ERR_SUCCESSS)
            return (-1);
        const ft_vector<ft_string> &keys = value->get_map_keys();

        if (value->get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = value->get_error();
            return (-1);
        }
        const ft_map<ft_string, yaml_value*> &map_reference = value->get_map();

        if (value->get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = value->get_error();
            return (-1);
        }
        size_t key_index;
        size_t key_count;

        key_index = 0;
        key_count = keys.size();
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];

            if (keys.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = keys.get_error();
                return (-1);
            }
            const char *key_cstr;

            key_cstr = key.c_str();
            if (!key_cstr)
            {
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (-1);
            }
            yaml_value *child_value;

            child_value = map_reference.at(key);
            if (map_reference.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = map_reference.get_error();
                return (-1);
            }
            ft_dom_node *child_node;

            child_node = new(std::nothrow) ft_dom_node();
            if (!child_node)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                return (-1);
            }
            if (child_node->set_name(key_cstr) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            if (yaml_dom_populate_node(child_value, child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            if (node->add_child(child_node) != 0)
            {
                yaml_dom_delete_node(child_node);
                return (-1);
            }
            key_index += 1;
        }
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int yaml_value_to_dom(const yaml_value *value, ft_dom_document &dom) noexcept
{
    if (!value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    dom.clear();
    if (dom.get_error() != FT_ERR_SUCCESSS)
        return (-1);
    ft_dom_node *root_node;

    root_node = new(std::nothrow) ft_dom_node();
    if (!root_node)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    if (root_node->set_name("yaml") != 0)
    {
        yaml_dom_delete_node(root_node);
        return (-1);
    }
    if (yaml_dom_populate_node(value, root_node) != 0)
    {
        yaml_dom_delete_node(root_node);
        return (-1);
    }
    dom.set_root(root_node);
    if (dom.get_error() != FT_ERR_SUCCESSS)
    {
        yaml_dom_delete_node(root_node);
        return (-1);
    }
    return (0);
}

static yaml_value *yaml_dom_build_value(ft_dom_node *node, int *status) noexcept
{
    if (status)
        *status = FT_ERR_SUCCESSS;
    if (!node)
    {
        if (status)
            *status = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_dom_node_type node_type;

    node_type = node->get_type();
    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        if (status)
            *status = node->get_error();
        ft_errno = node->get_error();
        return (ft_nullptr);
    }
    yaml_value *result;

    result = new(std::nothrow) yaml_value();
    if (!result)
    {
        if (status)
            *status = FT_ERR_NO_MEMORY;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (node_type == FT_DOM_NODE_VALUE)
    {
        const ft_string &scalar = node->get_value();

        if (node->get_error() != FT_ERR_SUCCESSS)
        {
            if (status)
                *status = node->get_error();
            delete result;
            ft_errno = node->get_error();
            return (ft_nullptr);
        }
        result->set_type(YAML_SCALAR);
        if (result->get_error() != FT_ERR_SUCCESSS)
        {
            int result_error;

            result_error = result->get_error();
            if (status)
                *status = result_error;
            delete result;
            ft_errno = result_error;
            return (ft_nullptr);
        }
        result->set_scalar(scalar);
        if (result->get_error() != FT_ERR_SUCCESSS)
        {
            int result_error;

            result_error = result->get_error();
            if (status)
                *status = result_error;
            delete result;
            ft_errno = result_error;
            return (ft_nullptr);
        }
        return (result);
    }
    if (node_type == FT_DOM_NODE_ARRAY)
    {
        const ft_vector<ft_dom_node*> &children = node->get_children();

        if (node->get_error() != FT_ERR_SUCCESSS)
        {
            if (status)
                *status = node->get_error();
            delete result;
            ft_errno = node->get_error();
            return (ft_nullptr);
        }
        size_t index;
        size_t count;

        index = 0;
        count = children.size();
        while (index < count)
        {
            ft_dom_node *child_node = children[index];

            if (children.get_error() != FT_ERR_SUCCESSS)
            {
                if (status)
                    *status = children.get_error();
                delete result;
                ft_errno = children.get_error();
                return (ft_nullptr);
            }
            int child_status;
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
            if (result->get_error() != FT_ERR_SUCCESSS)
            {
                int result_error;

                result_error = result->get_error();
                if (status)
                    *status = result_error;
                yaml_free(child_value);
                delete result;
                ft_errno = result_error;
                return (ft_nullptr);
            }
            index += 1;
        }
        return (result);
    }
    if (node_type == FT_DOM_NODE_OBJECT)
    {
        const ft_vector<ft_dom_node*> &children = node->get_children();

        if (node->get_error() != FT_ERR_SUCCESSS)
        {
            if (status)
                *status = node->get_error();
            delete result;
            ft_errno = node->get_error();
            return (ft_nullptr);
        }
        size_t index;
        size_t count;

        index = 0;
        count = children.size();
        while (index < count)
        {
            ft_dom_node *child_node = children[index];

            if (children.get_error() != FT_ERR_SUCCESSS)
            {
                if (status)
                    *status = children.get_error();
                delete result;
                ft_errno = children.get_error();
                return (ft_nullptr);
            }
            const ft_string &child_name = child_node->get_name();

            if (child_node->get_error() != FT_ERR_SUCCESSS)
            {
                if (status)
                    *status = child_node->get_error();
                delete result;
                ft_errno = child_node->get_error();
                return (ft_nullptr);
            }
            int child_status;
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
            if (result->get_error() != FT_ERR_SUCCESSS)
            {
                int result_error;

                result_error = result->get_error();
                if (status)
                    *status = result_error;
                yaml_free(child_value);
                delete result;
                ft_errno = result_error;
                return (ft_nullptr);
            }
            index += 1;
        }
        return (result);
    }
    if (status)
        *status = FT_ERR_INVALID_ARGUMENT;
    delete result;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (ft_nullptr);
}

yaml_value *yaml_value_from_dom(const ft_dom_document &dom) noexcept
{
    ft_dom_node *root_node;

    root_node = dom.get_root();
    if (dom.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = dom.get_error();
        return (ft_nullptr);
    }
    if (!root_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    int status;
    yaml_value *result;

    result = yaml_dom_build_value(root_node, &status);
    if (!result)
    {
        if (status == FT_ERR_SUCCESSS)
            status = FT_ERR_INVALID_ARGUMENT;
        ft_errno = status;
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}


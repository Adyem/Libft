#include "xml_dom_bridge.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <new>

static void xml_dom_delete_node(ft_dom_node *node) noexcept
{
    if (!node)
        return ;
    delete node;
    return ;
}

static int xml_dom_populate_node(const xml_node *source, ft_dom_node *target) noexcept;

static int xml_dom_populate_node_locked(const xml_node *source, ft_dom_node *target) noexcept
{
    const char *node_name;

    node_name = source->name;
    if (!node_name || node_name[0] == '\0')
        node_name = source->local_name ? source->local_name : "";
    if (target->set_name(node_name) != 0)
        return (-1);
    target->set_type(FT_DOM_NODE_ELEMENT);
    if (target->get_error() != FT_ERR_SUCCESSS)
        return (-1);
    if (source->namespace_prefix && source->namespace_prefix[0] != '\0')
    {
        if (target->add_attribute("xml:prefix", source->namespace_prefix) != 0)
            return (-1);
    }
    if (source->namespace_uri && source->namespace_uri[0] != '\0')
    {
        if (target->add_attribute("xml:ns", source->namespace_uri) != 0)
            return (-1);
    }
    if (source->text && source->text[0] != '\0')
    {
        if (target->set_value(source->text) != 0)
            return (-1);
    }
    ft_unordered_map<char*, char*>::const_iterator attribute_iterator = source->attributes.begin();
    if (source->attributes.last_operation_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = source->attributes.last_operation_error();
        return (-1);
    }
    ft_unordered_map<char*, char*>::const_iterator attribute_end = source->attributes.end();
    if (source->attributes.last_operation_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = source->attributes.last_operation_error();
        return (-1);
    }
    while (attribute_iterator != attribute_end)
    {
        const ft_pair<char*, char*> &entry = *attribute_iterator;
        const char *attribute_name;
        const char *attribute_value;

        attribute_name = entry.first ? entry.first : "";
        attribute_value = entry.second ? entry.second : "";
        if (target->add_attribute(attribute_name, attribute_value) != 0)
            return (-1);
        ++attribute_iterator;
    }
    size_t index;
    size_t count;

    index = 0;
    count = source->children.size();
    while (index < count)
    {
        xml_node *child_source;

        child_source = source->children[index];
        if (source->children.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = source->children.get_error();
            return (-1);
        }
        if (!child_source)
        {
            index += 1;
            continue ;
        }
        ft_dom_node *child_target;

        child_target = new(std::nothrow) ft_dom_node();
        if (!child_target)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (-1);
        }
        if (xml_dom_populate_node(child_source, child_target) != 0)
        {
            xml_dom_delete_node(child_target);
            return (-1);
        }
        if (target->add_child(child_target) != 0)
        {
            xml_dom_delete_node(child_target);
            return (-1);
        }
        index += 1;
    }
    return (0);
}

static int xml_dom_populate_node(const xml_node *source, ft_dom_node *target) noexcept
{
    bool lock_acquired;

    if (!source || !target)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (xml_node_lock(source, &lock_acquired) != 0)
        return (-1);
    int status;

    status = xml_dom_populate_node_locked(source, target);
    xml_node_unlock(source, lock_acquired);
    return (status);
}

int xml_document_to_dom(const xml_document &document, ft_dom_document &dom) noexcept
{
    dom.clear();
    if (dom.get_error() != FT_ERR_SUCCESSS)
        return (-1);
    xml_node *root_node;

    root_node = document.get_root();
    if (!root_node)
    {
        int document_error;

        document_error = document.get_error();
        if (document_error != FT_ERR_SUCCESSS)
            ft_errno = document_error;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_dom_node *dom_root;

    dom_root = new(std::nothrow) ft_dom_node();
    if (!dom_root)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    if (xml_dom_populate_node(root_node, dom_root) != 0)
    {
        xml_dom_delete_node(dom_root);
        return (-1);
    }
    dom.set_root(dom_root);
    if (dom.get_error() != FT_ERR_SUCCESSS)
    {
        xml_dom_delete_node(dom_root);
        return (-1);
    }
    return (0);
}

static int xml_dom_append_text(ft_string &output, const char *text) noexcept
{
    if (!text)
        return (0);
    output += text;
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    return (0);
}

static int xml_dom_serialize_node(ft_dom_node *node, ft_string &output) noexcept
{
    const ft_string &node_name = node->get_name();

    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = node->get_error();
        return (-1);
    }
    const char *name_cstr;

    name_cstr = node_name.c_str();
    if (!name_cstr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    output += "<";
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    output += node_name;
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    const ft_vector<ft_string> &attribute_keys = node->get_attribute_keys();

    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = node->get_error();
        return (-1);
    }
    const ft_vector<ft_string> &attribute_values = node->get_attribute_values();

    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = node->get_error();
        return (-1);
    }
    size_t attribute_index;
    size_t attribute_count;

    attribute_index = 0;
    attribute_count = attribute_keys.size();
    while (attribute_index < attribute_count)
    {
        const ft_string &attribute_name = attribute_keys[attribute_index];
        const ft_string &attribute_value = attribute_values[attribute_index];

        if (attribute_keys.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = attribute_keys.get_error();
            return (-1);
        }
        if (attribute_values.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = attribute_values.get_error();
            return (-1);
        }
        output += " ";
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        output += attribute_name;
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        output += "=\"";
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        output += attribute_value;
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        output += "\"";
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        attribute_index += 1;
    }
    const ft_vector<ft_dom_node*> &children = node->get_children();

    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = node->get_error();
        return (-1);
    }
    const ft_string &value = node->get_value();

    if (node->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = node->get_error();
        return (-1);
    }
    bool has_children;
    bool has_value;

    has_children = children.size() > 0;
    has_value = value.size() > 0;
    if (children.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = children.get_error();
        return (-1);
    }
    if (!has_children && !has_value)
    {
        output += "/>";
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        return (0);
    }
    output += ">";
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    if (has_value)
    {
        if (xml_dom_append_text(output, value.c_str()) != 0)
            return (-1);
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
            ft_errno = children.get_error();
            return (-1);
        }
        if (xml_dom_serialize_node(child_node, output) != 0)
            return (-1);
        index += 1;
    }
    output += "</";
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    output += node_name;
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    output += ">";
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    return (0);
}

int xml_document_from_dom(const ft_dom_document &dom, xml_document &document) noexcept
{
    ft_dom_node *root_node;

    root_node = dom.get_root();
    if (dom.get_error() != FT_ERR_SUCCESSS)
    {
        document.set_manual_error(dom.get_error());
        return (-1);
    }
    if (!root_node)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_string serialized;

    serialized = "";
    if (serialized.get_error() != FT_ERR_SUCCESSS)
    {
        document.set_manual_error(serialized.get_error());
        return (-1);
    }
    if (xml_dom_serialize_node(root_node, serialized) != 0)
    {
        document.set_manual_error(ft_errno);
        return (-1);
    }
    const char *xml_content;

    xml_content = serialized.c_str();
    if (!xml_content)
    {
        document.set_manual_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (document.load_from_string(xml_content) != 0)
    {
        document.set_manual_error(document.get_error());
        return (-1);
    }
    return (0);
}


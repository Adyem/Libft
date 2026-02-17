#include "xml_dom_bridge.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <new>

static int xml_dom_report_error(int error_code) noexcept
{
    (void)error_code;
    return (-1);
}

static void xml_dom_record_success(void) noexcept
{
    return ;
}

static int xml_dom_last_error(void) noexcept
{
    return (FT_ERR_INTERNAL);
}

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
    {
        const char *local_name = source->local_name;

        if (!local_name)
            local_name = "";
        node_name = local_name;
    }
    if (target->set_name(node_name) != 0)
        return (-1);
    target->set_type(FT_DOM_NODE_ELEMENT);
    if (target->get_error() != FT_ERR_SUCCESS)
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
    int attribute_error_code = source->attributes.last_operation_error();
    if (attribute_error_code != FT_ERR_SUCCESS)
        return (xml_dom_report_error(attribute_error_code));
    ft_unordered_map<char*, char*>::const_iterator attribute_end = source->attributes.end();
    attribute_error_code = source->attributes.last_operation_error();
    if (attribute_error_code != FT_ERR_SUCCESS)
        return (xml_dom_report_error(attribute_error_code));
    while (attribute_iterator != attribute_end)
    {
        const ft_pair<char*, char*> &entry = *attribute_iterator;
        const char *attribute_name;
        const char *attribute_value;

        attribute_name = entry.first;
        if (!attribute_name)
            attribute_name = "";
        attribute_value = entry.second;
        if (!attribute_value)
            attribute_value = "";
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
        int child_error_code;

        child_error_code = ft_vector<xml_node *>::last_operation_error();
        if (child_error_code != FT_ERR_SUCCESS)
            return (xml_dom_report_error(child_error_code));
        if (!child_source)
        {
            index += 1;
            continue ;
        }
        ft_dom_node *child_target;

        child_target = new(std::nothrow) ft_dom_node();
        if (!child_target)
            return (xml_dom_report_error(FT_ERR_NO_MEMORY));
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
    xml_dom_record_success();
    return (0);
}

static int xml_dom_populate_node(const xml_node *source, ft_dom_node *target) noexcept
{
    bool lock_acquired;

    if (!source || !target)
    {
        return (xml_dom_report_error(FT_ERR_INVALID_ARGUMENT));
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
    if (dom.get_error() != FT_ERR_SUCCESS)
        return (-1);
    xml_node *root_node;

    root_node = document.get_root();
    if (!root_node)
    {
        int document_error = document.get_error();

        if (document_error == FT_ERR_SUCCESS)
            document_error = FT_ERR_INVALID_ARGUMENT;
        return (xml_dom_report_error(document_error));
    }
    ft_dom_node *dom_root;

    dom_root = new(std::nothrow) ft_dom_node();
    if (!dom_root)
        return (xml_dom_report_error(FT_ERR_NO_MEMORY));
    if (xml_dom_populate_node(root_node, dom_root) != 0)
    {
        xml_dom_delete_node(dom_root);
        return (-1);
    }
    dom.set_root(dom_root);
    if (dom.get_error() != FT_ERR_SUCCESS)
    {
        xml_dom_delete_node(dom_root);
        return (xml_dom_report_error(dom.get_error()));
    }
    xml_dom_record_success();
    return (0);
}

static int xml_dom_check_string_error(const ft_string &value) noexcept
{
    int error_code;

    (void)value;
    error_code = ft_string::last_operation_error();
    return (error_code);
}

static int xml_dom_append_text(ft_string &output, const char *text) noexcept
{
    if (!text)
        return (0);
    output += text;
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

static int xml_dom_serialize_node(ft_dom_node *node, ft_string &output) noexcept
{
    const ft_string &node_name = node->get_name();

    if (node->get_error() != FT_ERR_SUCCESS)
        return (xml_dom_report_error(node->get_error()));
    const char *name_cstr;

    name_cstr = node_name.c_str();
    if (!name_cstr)
        return (xml_dom_report_error(FT_ERR_INVALID_ARGUMENT));
    output += "<";
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (-1);
    output += node_name;
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (-1);
    const ft_vector<ft_string> &attribute_keys = node->get_attribute_keys();

    if (node->get_error() != FT_ERR_SUCCESS)
        return (xml_dom_report_error(node->get_error()));
    const ft_vector<ft_string> &attribute_values = node->get_attribute_values();

    if (node->get_error() != FT_ERR_SUCCESS)
        return (xml_dom_report_error(node->get_error()));
    size_t attribute_index;
    size_t attribute_count;

    attribute_index = 0;
    attribute_count = attribute_keys.size();
    while (attribute_index < attribute_count)
    {
        const ft_string &attribute_name = attribute_keys[attribute_index];
        const ft_string &attribute_value = attribute_values[attribute_index];

        int attribute_keys_error;

        attribute_keys_error = ft_vector<ft_string>::last_operation_error();
        if (attribute_keys_error != FT_ERR_SUCCESS)
            return (xml_dom_report_error(attribute_keys_error));
        int attribute_values_error;

        attribute_values_error = ft_vector<ft_string>::last_operation_error();
        if (attribute_values_error != FT_ERR_SUCCESS)
            return (xml_dom_report_error(attribute_values_error));
        output += " ";
        if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
            return (-1);
        output += attribute_name;
        if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
            return (-1);
        output += "=\"";
        if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
            return (-1);
        output += attribute_value;
        if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
            return (-1);
        output += "\"";
        if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
            return (-1);
        attribute_index += 1;
    }
    const ft_vector<ft_dom_node*> &children = node->get_children();

    if (node->get_error() != FT_ERR_SUCCESS)
        return (xml_dom_report_error(node->get_error()));
    const ft_string &value = node->get_value();

    if (node->get_error() != FT_ERR_SUCCESS)
        return (xml_dom_report_error(node->get_error()));
    bool has_children;
    bool has_value;

    has_children = children.size() > 0;
    has_value = value.size() > 0;
    int children_error_code;

    children_error_code = ft_vector<ft_dom_node *>::last_operation_error();
    if (children_error_code != FT_ERR_SUCCESS)
        return (xml_dom_report_error(children_error_code));
    if (!has_children && !has_value)
    {
    output += "/>";
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
    }
    output += ">";
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (xml_dom_report_error(xml_dom_last_error()));
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

        int child_vector_error;

        child_vector_error = ft_vector<ft_dom_node *>::last_operation_error();
        if (child_vector_error != FT_ERR_SUCCESS)
            return (xml_dom_report_error(child_vector_error));
        if (xml_dom_serialize_node(child_node, output) != 0)
            return (-1);
        index += 1;
    }
    output += "</";
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (xml_dom_report_error(xml_dom_last_error()));
    output += node_name;
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (xml_dom_report_error(xml_dom_last_error()));
    output += ">";
    if (xml_dom_check_string_error(output) != FT_ERR_SUCCESS)
        return (xml_dom_report_error(xml_dom_last_error()));
    xml_dom_record_success();
    return (0);
}

int xml_document_from_dom(const ft_dom_document &dom, xml_document &document) noexcept
{
    ft_dom_node *root_node;

    root_node = dom.get_root();
    if (dom.get_error() != FT_ERR_SUCCESS)
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
    int string_error = xml_dom_check_string_error(serialized);
    if (string_error != FT_ERR_SUCCESS)
    {
        document.set_manual_error(string_error);
        return (-1);
    }
    if (xml_dom_serialize_node(root_node, serialized) != 0)
    {
        document.set_manual_error(xml_dom_last_error());
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

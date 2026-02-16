#include "parser.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

struct html_string_builder
{
    char *buffer;
    size_t length;
    size_t capacity;
};

static int html_builder_initialize(html_string_builder *builder)
{
    builder->capacity = 64;
    builder->length = 0;
    builder->buffer = static_cast<char *>(cma_malloc(builder->capacity));
    if (builder->buffer == ft_nullptr)
        return (-1);
    builder->buffer[0] = '\0';
    return (0);
}

static int html_builder_reserve(html_string_builder *builder, size_t additional_length)
{
    size_t required_capacity;
    size_t new_capacity;
    char *new_buffer;

    required_capacity = builder->length + additional_length + 1;
    if (required_capacity <= builder->capacity)
        return (0);
    new_capacity = builder->capacity;
    while (new_capacity < required_capacity)
    {
        if (new_capacity > static_cast<size_t>(-1) / 2)
            return (-1);
        new_capacity *= 2;
    }
    new_buffer = static_cast<char *>(cma_realloc(builder->buffer, new_capacity));
    if (new_buffer == ft_nullptr)
        return (-1);
    builder->buffer = new_buffer;
    builder->capacity = new_capacity;
    return (0);
}

static int html_builder_append_char(html_string_builder *builder, char character)
{
    if (html_builder_reserve(builder, 1) != 0)
        return (-1);
    builder->buffer[builder->length] = character;
    builder->length += 1;
    builder->buffer[builder->length] = '\0';
    return (0);
}

static int html_builder_append_c_string(html_string_builder *builder, const char *value)
{
    size_t index;

    if (value == ft_nullptr)
        return (0);
    index = 0;
    while (value[index] != '\0')
        index += 1;
    if (html_builder_reserve(builder, index) != 0)
        return (-1);
    index = 0;
    while (value[index] != '\0')
    {
        builder->buffer[builder->length] = value[index];
        builder->length += 1;
        index += 1;
    }
    builder->buffer[builder->length] = '\0';
    return (0);
}

static int html_builder_append_indent(html_string_builder *builder, int indent_level)
{
    int indent_index;

    indent_index = 0;
    while (indent_index < indent_level)
    {
        if (html_builder_append_c_string(builder, "  ") != 0)
            return (-1);
        indent_index += 1;
    }
    return (0);
}

static int html_builder_append_attributes(html_string_builder *builder, html_attr *attribute)
{
    while (attribute != ft_nullptr)
    {
        bool lock_acquired;
        int lock_status;
        html_attr *next_attribute;

        lock_acquired = false;
        lock_status = html_attr_lock(attribute, &lock_acquired);
        if (lock_status != 0)
            return (-1);
        next_attribute = attribute->next;
        if (attribute->key != ft_nullptr && attribute->value != ft_nullptr)
        {
            if (html_builder_append_char(builder, ' ') != 0)
            {
                html_attr_unlock(attribute, lock_acquired);
                return (-1);
            }
            if (html_builder_append_c_string(builder, attribute->key) != 0)
            {
                html_attr_unlock(attribute, lock_acquired);
                return (-1);
            }
            if (html_builder_append_c_string(builder, "=\"") != 0)
            {
                html_attr_unlock(attribute, lock_acquired);
                return (-1);
            }
            if (html_builder_append_c_string(builder, attribute->value) != 0)
            {
                html_attr_unlock(attribute, lock_acquired);
                return (-1);
            }
            if (html_builder_append_char(builder, '"') != 0)
            {
                html_attr_unlock(attribute, lock_acquired);
                return (-1);
            }
        }
        html_attr_unlock(attribute, lock_acquired);
        attribute = next_attribute;
    }
    return (0);
}

static int html_builder_append_node(html_string_builder *builder, html_node *node, int indent_level)
{
    bool lock_acquired;
    int lock_status;
    int result;

    lock_acquired = false;
    lock_status = html_node_lock(node, &lock_acquired);
    if (lock_status != 0)
        return (-1);
    result = html_builder_append_indent(builder, indent_level);
    if (result != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (html_builder_append_char(builder, '<') != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (html_builder_append_c_string(builder, node->tag) != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (html_builder_append_attributes(builder, node->attributes) != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (node->text == ft_nullptr && node->children == ft_nullptr)
    {
        if (html_builder_append_c_string(builder, "/>\n") != 0)
        {
            html_node_unlock(node, lock_acquired);
            return (-1);
        }
        html_node_unlock(node, lock_acquired);
        return (0);
    }
    if (html_builder_append_char(builder, '>') != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (node->text != ft_nullptr)
    {
        if (html_builder_append_c_string(builder, node->text) != 0)
        {
            html_node_unlock(node, lock_acquired);
            return (-1);
        }
    }
    if (node->children != ft_nullptr)
    {
        html_node *child;

        if (html_builder_append_char(builder, '\n') != 0)
        {
            html_node_unlock(node, lock_acquired);
            return (-1);
        }
        child = node->children;
        while (child != ft_nullptr)
        {
            html_node *next_child;

            next_child = child->next;
            if (html_builder_append_node(builder, child, indent_level + 1) != 0)
            {
                html_node_unlock(node, lock_acquired);
                return (-1);
            }
            child = next_child;
        }
        if (html_builder_append_indent(builder, indent_level) != 0)
        {
            html_node_unlock(node, lock_acquired);
            return (-1);
        }
    }
    if (html_builder_append_c_string(builder, "</") != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (html_builder_append_c_string(builder, node->tag) != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    if (html_builder_append_c_string(builder, ">\n") != 0)
    {
        html_node_unlock(node, lock_acquired);
        return (-1);
    }
    html_node_unlock(node, lock_acquired);
    return (0);
}

char *html_write_to_string(html_node *node_list)
{
    html_string_builder builder;
    html_node *current_node;

    if (html_builder_initialize(&builder) != 0)
        return (ft_nullptr);
    current_node = node_list;
    while (current_node != ft_nullptr)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (html_builder_append_node(&builder, current_node, 0) != 0)
        {
            cma_free(builder.buffer);
            return (ft_nullptr);
        }
        current_node = next_node;
    }
    return (builder.buffer);
}

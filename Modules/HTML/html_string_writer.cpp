#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"

struct html_string_builder
{
    char *buffer;
    ft_size_t length;
    ft_size_t capacity;
};

static int32_t html_builder_initialize(html_string_builder *builder)
{
    builder->capacity = 64;
    builder->length = 0;
    builder->buffer = static_cast<char *>(cma_malloc(builder->capacity));
    if (builder->buffer == ft_nullptr)
        return (FT_ERR_INTERNAL);
    builder->buffer[0] = '\0';
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_reserve(html_string_builder *builder, ft_size_t additional_length)
{
    ft_size_t required_capacity;
    ft_size_t new_capacity;
    char *new_buffer;

    required_capacity = builder->length + additional_length + 1;
    if (required_capacity <= builder->capacity)
        return (FT_ERR_SUCCESS);
    new_capacity = builder->capacity;
    while (new_capacity < required_capacity)
    {
        if (new_capacity > static_cast<ft_size_t>(-1) / 2)
            return (FT_ERR_INTERNAL);
        new_capacity *= 2;
    }
    new_buffer = static_cast<char *>(cma_realloc(builder->buffer, new_capacity));
    if (new_buffer == ft_nullptr)
        return (FT_ERR_INTERNAL);
    builder->buffer = new_buffer;
    builder->capacity = new_capacity;
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_append_char(html_string_builder *builder, char character)
{
    if (html_builder_reserve(builder, 1) != 0)
        return (FT_ERR_INTERNAL);
    builder->buffer[builder->length] = character;
    builder->length += 1;
    builder->buffer[builder->length] = '\0';
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_append_c_string(html_string_builder *builder, const char *value)
{
    ft_size_t index;

    if (value == ft_nullptr)
        return (FT_ERR_SUCCESS);
    index = 0;
    while (value[index] != '\0')
        index += 1;
    if (html_builder_reserve(builder, index) != 0)
        return (FT_ERR_INTERNAL);
    index = 0;
    while (value[index] != '\0')
    {
        builder->buffer[builder->length] = value[index];
        builder->length += 1;
        index += 1;
    }
    builder->buffer[builder->length] = '\0';
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_append_indent(html_string_builder *builder, int32_t indent_level)
{
    int32_t indent_index;

    indent_index = 0;
    while (indent_index < indent_level)
    {
        if (html_builder_append_c_string(builder, "  ") != 0)
            return (FT_ERR_INTERNAL);
        indent_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_append_attributes(html_string_builder *builder, html_attr *attribute)
{
    while (attribute != ft_nullptr)
    {
        html_attr *next_attribute;

        next_attribute = attribute->next;
        if (attribute->key != ft_nullptr && attribute->value != ft_nullptr)
        {
            if (html_builder_append_char(builder, ' ') != 0)
                return (FT_ERR_INTERNAL);
            if (html_builder_append_c_string(builder, attribute->key) != 0)
                return (FT_ERR_INTERNAL);
            if (html_builder_append_c_string(builder, "=\"") != 0)
                return (FT_ERR_INTERNAL);
            if (html_builder_append_c_string(builder, attribute->value) != 0)
                return (FT_ERR_INTERNAL);
            if (html_builder_append_char(builder, '"') != 0)
                return (FT_ERR_INTERNAL);
        }
        attribute = next_attribute;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t html_builder_append_node(html_string_builder *builder, html_node *node, int32_t indent_level)
{
    int32_t result;

    result = html_builder_append_indent(builder, indent_level);
    if (result != 0)
        return (FT_ERR_INTERNAL);
    if (html_builder_append_char(builder, '<') != 0)
        return (FT_ERR_INTERNAL);
    if (html_builder_append_c_string(builder, node->tag) != 0)
        return (FT_ERR_INTERNAL);
    if (html_builder_append_attributes(builder, node->attributes) != 0)
        return (FT_ERR_INTERNAL);
    if (node->text == ft_nullptr && node->children == ft_nullptr)
    {
        if (html_builder_append_c_string(builder, "/>\n") != 0)
            return (FT_ERR_INTERNAL);
        return (FT_ERR_SUCCESS);
    }
    if (html_builder_append_char(builder, '>') != 0)
        return (FT_ERR_INTERNAL);
    if (node->text != ft_nullptr)
    {
        if (html_builder_append_c_string(builder, node->text) != 0)
            return (FT_ERR_INTERNAL);
    }
    if (node->children != ft_nullptr)
    {
        html_node *child;

        if (html_builder_append_char(builder, '\n') != 0)
            return (FT_ERR_INTERNAL);
        child = node->children;
        while (child != ft_nullptr)
        {
            html_node *next_child;

            next_child = child->next;
            if (html_builder_append_node(builder, child, indent_level + 1) != 0)
                return (FT_ERR_INTERNAL);
            child = next_child;
        }
        if (html_builder_append_indent(builder, indent_level) != 0)
            return (FT_ERR_INTERNAL);
    }
    if (html_builder_append_c_string(builder, "</") != 0)
        return (FT_ERR_INTERNAL);
    if (html_builder_append_c_string(builder, node->tag) != 0)
        return (FT_ERR_INTERNAL);
    if (html_builder_append_c_string(builder, ">\n") != 0)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
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

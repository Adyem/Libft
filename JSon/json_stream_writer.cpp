#include "json_stream_writer.hpp"

#include "json_stream_events.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int json_stream_writer_write_chunk(json_stream_writer *writer,
    const char *buffer,
    size_t size)
{
    size_t offset;
    size_t written;

    if (!writer || !buffer)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    offset = 0;
    while (offset < size)
    {
        written = writer->write_callback(writer->user_data, buffer + offset, size - offset);
        if (written == 0)
        {
            ft_errno = FT_ERR_IO;
            return (-1);
        }
        offset += written;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int json_stream_writer_write_literal(json_stream_writer *writer,
    const char *literal)
{
    size_t length;

    if (!literal)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    length = ft_strlen_size_t(literal);
    return (json_stream_writer_write_chunk(writer, literal, length));
}

static int json_stream_writer_write_escaped_string(json_stream_writer *writer,
    const char *value,
    size_t length)
{
    size_t index;
    unsigned char character;
    char buffer[6];
    const char *hex_digits;

    if (json_stream_writer_write_chunk(writer, "\"", 1) != 0)
        return (-1);
    index = 0;
    hex_digits = "0123456789ABCDEF";
    while (index < length)
    {
        character = static_cast<unsigned char>(value[index]);
        if (character == '\\' || character == '"')
        {
            buffer[0] = '\\';
            buffer[1] = static_cast<char>(character);
            if (json_stream_writer_write_chunk(writer, buffer, 2) != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character == '\b')
        {
            if (json_stream_writer_write_literal(writer, "\\b") != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character == '\f')
        {
            if (json_stream_writer_write_literal(writer, "\\f") != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character == '\n')
        {
            if (json_stream_writer_write_literal(writer, "\\n") != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character == '\r')
        {
            if (json_stream_writer_write_literal(writer, "\\r") != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character == '\t')
        {
            if (json_stream_writer_write_literal(writer, "\\t") != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (character < 0x20)
        {
            buffer[0] = '\\';
            buffer[1] = 'u';
            buffer[2] = '0';
            buffer[3] = '0';
            buffer[4] = hex_digits[(character >> 4) & 0x0F];
            buffer[5] = hex_digits[character & 0x0F];
            if (json_stream_writer_write_chunk(writer, buffer, 6) != 0)
                return (-1);
            index += 1;
            continue ;
        }
        buffer[0] = static_cast<char>(character);
        if (json_stream_writer_write_chunk(writer, buffer, 1) != 0)
            return (-1);
        index += 1;
    }
    if (json_stream_writer_write_chunk(writer, "\"", 1) != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int json_stream_writer_write_string(json_stream_writer *writer,
    const json_stream_scalar *scalar)
{
    if (!scalar)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (scalar->data == ft_nullptr && scalar->length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (scalar->data == ft_nullptr)
        return (json_stream_writer_write_chunk(writer, "\"\"", 2));
    return (json_stream_writer_write_escaped_string(writer, scalar->data, scalar->length));
}

static int json_stream_writer_write_number(json_stream_writer *writer,
    const json_stream_scalar *scalar)
{
    if (!scalar || scalar->data == ft_nullptr || scalar->length == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (json_stream_writer_write_chunk(writer, scalar->data, scalar->length));
}

static json_stream_writer_context *json_stream_writer_current_context(json_stream_writer *writer)
{
    if (!writer)
        return (ft_nullptr);
    if (writer->context_size == 0)
        return (ft_nullptr);
    return (&writer->context_stack[writer->context_size - 1]);
}

static int json_stream_writer_ensure_capacity(json_stream_writer *writer,
    size_t required)
{
    size_t capacity;
    size_t new_capacity;
    json_stream_writer_context *resized;

    if (!writer)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (writer->context_capacity >= required)
        return (0);
    capacity = writer->context_capacity;
    if (capacity == 0)
        capacity = 4;
    new_capacity = capacity;
    while (new_capacity < required)
    {
        size_t next_capacity;

        next_capacity = new_capacity * 2;
        if (next_capacity <= new_capacity)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            return (-1);
        }
        new_capacity = next_capacity;
    }
    if (writer->context_stack == ft_nullptr)
        resized = static_cast<json_stream_writer_context *>(cma_malloc(new_capacity * sizeof(*resized)));
    else
        resized = static_cast<json_stream_writer_context *>(cma_realloc(writer->context_stack, new_capacity * sizeof(*resized)));
    if (!resized)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    writer->context_stack = resized;
    writer->context_capacity = new_capacity;
    return (0);
}

static int json_stream_writer_push_context(json_stream_writer *writer,
    char type)
{
    json_stream_writer_context *context;

    if (type != 'o' && type != 'a')
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (json_stream_writer_ensure_capacity(writer, writer->context_size + 1) != 0)
        return (-1);
    context = &writer->context_stack[writer->context_size];
    context->type = type;
    context->need_comma = false;
    if (type == 'o')
        context->expecting_key = true;
    else
        context->expecting_key = false;
    writer->context_size += 1;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int json_stream_writer_pop_context(json_stream_writer *writer,
    char expected_type)
{
    json_stream_writer_context *context;

    if (!writer || writer->context_size == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    context = &writer->context_stack[writer->context_size - 1];
    if (context->type != expected_type)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (expected_type == 'o' && context->expecting_key == false)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    writer->context_size -= 1;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int json_stream_writer_prepare_value(json_stream_writer *writer)
{
    json_stream_writer_context *current;

    current = json_stream_writer_current_context(writer);
    if (current == ft_nullptr)
    {
        if (writer->root_value_emitted != false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (-1);
        }
        writer->root_value_emitted = true;
        return (0);
    }
    if (current->type == 'o')
    {
        if (current->expecting_key != false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (-1);
        }
        return (0);
    }
    if (current->type == 'a')
    {
        if (current->need_comma != false)
        {
            if (json_stream_writer_write_literal(writer, ",") != 0)
                return (-1);
        }
        return (0);
    }
    ft_errno = FT_ERR_INVALID_STATE;
    return (-1);
}

static void json_stream_writer_mark_parent_consumed(json_stream_writer *writer)
{
    json_stream_writer_context *current;

    current = json_stream_writer_current_context(writer);
    if (current == ft_nullptr)
        return ;
    if (current->type == 'o')
    {
        current->expecting_key = true;
        current->need_comma = true;
        return ;
    }
    if (current->type == 'a')
    {
        current->need_comma = true;
        return ;
    }
    return ;
}

int json_stream_writer_init(json_stream_writer *writer,
    json_stream_write_callback callback,
    void *user_data)
{
    if (!writer || !callback)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_bzero(writer, sizeof(*writer));
    writer->write_callback = callback;
    writer->user_data = user_data;
    writer->context_stack = ft_nullptr;
    writer->context_size = 0;
    writer->context_capacity = 0;
    writer->began_document = false;
    writer->finished_document = false;
    writer->root_value_emitted = false;
    writer->error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void json_stream_writer_destroy(json_stream_writer *writer)
{
    if (!writer)
        return ;
    if (writer->context_stack != ft_nullptr)
        cma_free(writer->context_stack);
    writer->context_stack = ft_nullptr;
    writer->context_size = 0;
    writer->context_capacity = 0;
    writer->write_callback = ft_nullptr;
    writer->user_data = ft_nullptr;
    writer->began_document = false;
    writer->finished_document = false;
    writer->root_value_emitted = false;
    writer->error_code = FT_ERR_SUCCESSS;
    return ;
}

static int json_stream_writer_write_boolean(json_stream_writer *writer,
    bool value)
{
    if (value != false)
        return (json_stream_writer_write_literal(writer, "true"));
    return (json_stream_writer_write_literal(writer, "false"));
}

int json_stream_writer_process(json_stream_writer *writer,
    const json_stream_event *event)
{
    json_stream_writer_context *current;

    if (!writer || !event)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!writer->write_callback)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        writer->error_code = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (writer->error_code != FT_ERR_SUCCESSS)
    {
        ft_errno = writer->error_code;
        return (-1);
    }
    if (event->type == JSON_STREAM_EVENT_BEGIN_DOCUMENT)
    {
        if (writer->began_document != false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            writer->error_code = FT_ERR_INVALID_STATE;
            return (-1);
        }
        writer->began_document = true;
        writer->finished_document = false;
        writer->root_value_emitted = false;
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_END_DOCUMENT)
    {
        if (writer->began_document == false || writer->finished_document != false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            writer->error_code = FT_ERR_INVALID_STATE;
            return (-1);
        }
        if (writer->context_size != 0 || writer->root_value_emitted == false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            writer->error_code = FT_ERR_INVALID_STATE;
            return (-1);
        }
        writer->finished_document = true;
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (writer->began_document == false || writer->finished_document != false)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        writer->error_code = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (event->type == JSON_STREAM_EVENT_KEY)
    {
        if (!event->value.data && event->value.length != 0)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            writer->error_code = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        current = json_stream_writer_current_context(writer);
        if (current == ft_nullptr || current->type != 'o' || current->expecting_key == false)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            writer->error_code = FT_ERR_INVALID_STATE;
            return (-1);
        }
        if (current->need_comma != false)
        {
            if (json_stream_writer_write_literal(writer, ",") != 0)
            {
                writer->error_code = ft_errno;
                return (-1);
            }
        }
        if (event->value.data == ft_nullptr)
        {
            if (json_stream_writer_write_chunk(writer, "\"\"", 2) != 0)
            {
                writer->error_code = ft_errno;
                return (-1);
            }
        }
        else if (json_stream_writer_write_escaped_string(writer,
                 event->value.data,
                 event->value.length) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, ":") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        current->expecting_key = false;
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_BEGIN_OBJECT)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, "{") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        if (json_stream_writer_push_context(writer, 'o') != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_END_OBJECT)
    {
        if (json_stream_writer_pop_context(writer, 'o') != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, "}") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_BEGIN_ARRAY)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, "[") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        if (json_stream_writer_push_context(writer, 'a') != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_END_ARRAY)
    {
        if (json_stream_writer_pop_context(writer, 'a') != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, "]") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_STRING)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_string(writer, &event->value) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_NUMBER)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_number(writer, &event->value) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_BOOLEAN)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_boolean(writer, event->bool_value) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (event->type == JSON_STREAM_EVENT_NULL)
    {
        if (json_stream_writer_prepare_value(writer) != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        if (json_stream_writer_write_literal(writer, "null") != 0)
        {
            writer->error_code = ft_errno;
            return (-1);
        }
        json_stream_writer_mark_parent_consumed(writer);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    writer->error_code = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int json_stream_writer_finish(json_stream_writer *writer)
{
    if (!writer)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (writer->error_code != FT_ERR_SUCCESSS)
    {
        ft_errno = writer->error_code;
        return (-1);
    }
    if (writer->began_document == false || writer->finished_document == false)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (writer->context_size != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

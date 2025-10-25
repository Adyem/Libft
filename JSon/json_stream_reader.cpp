#include "json_stream_reader.hpp"

#include "json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

#include <cctype>

static const size_t JSON_STREAM_READ_ERROR = static_cast<size_t>(-1);

#define JSON_STREAM_STATUS_OK 0
#define JSON_STREAM_STATUS_END 1
#define JSON_STREAM_STATUS_ERROR -1

static size_t json_stream_reader_file_callback(void *user_data, char *buffer, size_t max_size)
{
    FILE *file = static_cast<FILE *>(user_data);

    if (!file || !buffer || max_size == 0)
        return (0);
    size_t bytes_read = fread(buffer, 1, max_size, file);
    if (bytes_read == 0)
    {
        if (ferror(file))
            return (JSON_STREAM_READ_ERROR);
    }
    return (bytes_read);
}

static int json_stream_reader_fill(json_stream_reader *reader)
{
    if (!reader)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (JSON_STREAM_STATUS_ERROR);
    }
    if (reader->buffer_index < reader->buffer_size)
        return (JSON_STREAM_STATUS_OK);
    if (reader->end_of_stream)
        return (JSON_STREAM_STATUS_END);
    size_t bytes_read = reader->read_callback(reader->user_data, reader->buffer, reader->buffer_capacity);
    if (bytes_read == JSON_STREAM_READ_ERROR)
    {
        reader->error_code = FT_ERR_IO;
        ft_errno = FT_ERR_IO;
        return (JSON_STREAM_STATUS_ERROR);
    }
    if (bytes_read == 0)
    {
        reader->end_of_stream = true;
        return (JSON_STREAM_STATUS_END);
    }
    reader->buffer_index = 0;
    reader->buffer_size = bytes_read;
    return (JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_peek(json_stream_reader *reader, char *out_char)
{
    if (!reader || !out_char)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (JSON_STREAM_STATUS_ERROR);
    }
    int status = json_stream_reader_fill(reader);
    if (status != JSON_STREAM_STATUS_OK)
        return (status);
    *out_char = reader->buffer[reader->buffer_index];
    return (JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_consume(json_stream_reader *reader, char *out_char)
{
    int status = json_stream_reader_peek(reader, out_char);
    if (status != JSON_STREAM_STATUS_OK)
        return (status);
    reader->buffer_index += 1;
    return (JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_expect(json_stream_reader *reader, char expected_char)
{
    char current_char;
    int status = json_stream_reader_consume(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (current_char != expected_char)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (0);
}

static int json_stream_skip_whitespace(json_stream_reader *reader)
{
    char current_char;
    int status = json_stream_reader_peek(reader, &current_char);
    while (status == JSON_STREAM_STATUS_OK
        && ft_isspace(static_cast<unsigned char>(current_char)))
    {
        json_stream_reader_consume(reader, &current_char);
        status = json_stream_reader_peek(reader, &current_char);
    }
    return (status);
}

static int json_stream_ensure_capacity(char **buffer, size_t *capacity, size_t minimum)
{
    if (!buffer || !capacity)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (*capacity > minimum)
        return (0);
    size_t new_capacity = *capacity;
    if (new_capacity == 0)
        new_capacity = 1;
    while (new_capacity <= minimum)
    {
        size_t next_capacity = new_capacity * 2;
        if (next_capacity <= new_capacity)
            next_capacity = new_capacity + 1;
        new_capacity = next_capacity;
    }
    char *resized_buffer = static_cast<char *>(cma_realloc(*buffer, new_capacity));
    if (!resized_buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    *buffer = resized_buffer;
    *capacity = new_capacity;
    return (0);
}

static int json_stream_hex_value(char character, unsigned int &value) noexcept
{
    if (character >= '0' && character <= '9')
    {
        value = static_cast<unsigned int>(character - '0');
        return (0);
    }
    if (character >= 'a' && character <= 'f')
    {
        value = static_cast<unsigned int>(character - 'a' + 10);
        return (0);
    }
    if (character >= 'A' && character <= 'F')
    {
        value = static_cast<unsigned int>(character - 'A' + 10);
        return (0);
    }
    return (-1);
}

static int json_stream_append_utf8(char **buffer,
    size_t *capacity,
    size_t *length,
    unsigned int code_point) noexcept
{
    if (!buffer || !capacity || !length)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (code_point <= 0x7F)
    {
        if (json_stream_ensure_capacity(buffer, capacity, *length + 1) != 0)
            return (-1);
        (*buffer)[*length] = static_cast<char>(code_point);
        *length += 1;
        return (0);
    }
    if (code_point <= 0x7FF)
    {
        if (json_stream_ensure_capacity(buffer, capacity, *length + 2) != 0)
            return (-1);
        (*buffer)[*length] = static_cast<char>(0xC0 | (code_point >> 6));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        return (0);
    }
    if (code_point <= 0xFFFF)
    {
        if (json_stream_ensure_capacity(buffer, capacity, *length + 3) != 0)
            return (-1);
        (*buffer)[*length] = static_cast<char>(0xE0 | (code_point >> 12));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        return (0);
    }
    if (code_point <= 0x10FFFF)
    {
        if (json_stream_ensure_capacity(buffer, capacity, *length + 4) != 0)
            return (-1);
        (*buffer)[*length] = static_cast<char>(0xF0 | (code_point >> 18));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

static int json_stream_parse_code_unit(json_stream_reader *reader, unsigned int &code_unit)
{
    code_unit = 0;
    size_t digit_index = 0;
    while (digit_index < 4)
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        unsigned int digit_value = 0;
        if (json_stream_hex_value(current_char, digit_value) != 0)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        code_unit = (code_unit << 4) | digit_value;
        digit_index += 1;
    }
    return (0);
}

static int json_stream_append_escape(json_stream_reader *reader,
    char **buffer,
    size_t *capacity,
    size_t *length)
{
    char escape_char;
    int status = json_stream_reader_consume(reader, &escape_char);
    if (status != JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (escape_char == '"' || escape_char == '\\' || escape_char == '/')
    {
        if (json_stream_ensure_capacity(buffer, capacity, *length + 1) != 0)
            return (-1);
        (*buffer)[*length] = escape_char;
        *length += 1;
        return (0);
    }
    if (escape_char == 'b')
        return (json_stream_append_utf8(buffer, capacity, length, '\b'));
    if (escape_char == 'f')
        return (json_stream_append_utf8(buffer, capacity, length, '\f'));
    if (escape_char == 'n')
        return (json_stream_append_utf8(buffer, capacity, length, '\n'));
    if (escape_char == 'r')
        return (json_stream_append_utf8(buffer, capacity, length, '\r'));
    if (escape_char == 't')
        return (json_stream_append_utf8(buffer, capacity, length, '\t'));
    if (escape_char != 'u')
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    unsigned int code_unit = 0;
    if (json_stream_parse_code_unit(reader, code_unit) != 0)
        return (-1);
    unsigned int code_point = code_unit;
    if (code_unit >= 0xD800 && code_unit <= 0xDBFF)
    {
        char next_char;
        status = json_stream_reader_peek(reader, &next_char);
        if (status != JSON_STREAM_STATUS_OK || next_char != '\\')
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        status = json_stream_reader_consume(reader, &next_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        status = json_stream_reader_consume(reader, &next_char);
        if (status != JSON_STREAM_STATUS_OK || next_char != 'u')
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        unsigned int low_unit = 0;
        if (json_stream_parse_code_unit(reader, low_unit) != 0)
            return (-1);
        if (low_unit < 0xDC00 || low_unit > 0xDFFF)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        code_point = 0x10000;
        code_point = code_point + ((code_unit - 0xD800) << 10);
        code_point = code_point + (low_unit - 0xDC00);
    }
    else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (json_stream_append_utf8(buffer, capacity, length, code_point));
}

static char *json_stream_parse_string(json_stream_reader *reader)
{
    if (json_stream_reader_expect(reader, '"') != 0)
        return (ft_nullptr);
    size_t capacity = 32;
    size_t length = 0;
    char *result = static_cast<char *>(cma_malloc(capacity));
    if (!result)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    bool closed = false;
    while (!closed)
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            cma_free(result);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (current_char == '"')
        {
            closed = true;
            break;
        }
        if (current_char == '\\')
        {
            if (json_stream_append_escape(reader, &result, &capacity, &length) != 0)
            {
                cma_free(result);
                return (ft_nullptr);
            }
            continue;
        }
        if (static_cast<unsigned char>(current_char) < 0x20)
        {
            cma_free(result);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (json_stream_ensure_capacity(&result, &capacity, length + 1) != 0)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        result[length] = current_char;
        length += 1;
    }
    if (!closed)
    {
        cma_free(result);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (json_stream_ensure_capacity(&result, &capacity, length + 1) != 0)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    result[length] = '\0';
    ft_errno = ER_SUCCESS;
    return (result);
}

static int json_stream_match_literal(json_stream_reader *reader, const char *literal)
{
    size_t index = 0;
    while (literal[index])
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        if (current_char != literal[index])
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        index += 1;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static char *json_stream_parse_number(json_stream_reader *reader)
{
    size_t capacity = 32;
    size_t length = 0;
    char *number = static_cast<char *>(cma_malloc(capacity));
    if (!number)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    bool has_digits = false;
    char current_char;
    int status = json_stream_reader_peek(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
    {
        cma_free(number);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (current_char == '-' || current_char == '+')
    {
        if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
        {
            cma_free(number);
            return (ft_nullptr);
        }
        json_stream_reader_consume(reader, &current_char);
        number[length] = current_char;
        length += 1;
        status = json_stream_reader_peek(reader, &current_char);
    }
    while (status == JSON_STREAM_STATUS_OK
        && ft_isdigit(static_cast<unsigned char>(current_char)))
    {
        if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
        {
            cma_free(number);
            return (ft_nullptr);
        }
        json_stream_reader_consume(reader, &current_char);
        number[length] = current_char;
        length += 1;
        has_digits = true;
        status = json_stream_reader_peek(reader, &current_char);
    }
    if (status == JSON_STREAM_STATUS_OK && current_char == '.')
    {
        if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
        {
            cma_free(number);
            return (ft_nullptr);
        }
        json_stream_reader_consume(reader, &current_char);
        number[length] = current_char;
        length += 1;
        status = json_stream_reader_peek(reader, &current_char);
        while (status == JSON_STREAM_STATUS_OK
            && ft_isdigit(static_cast<unsigned char>(current_char)))
        {
            if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
            {
                cma_free(number);
                return (ft_nullptr);
            }
            json_stream_reader_consume(reader, &current_char);
            number[length] = current_char;
            length += 1;
            status = json_stream_reader_peek(reader, &current_char);
        }
    }
    if (status == JSON_STREAM_STATUS_OK
        && (current_char == 'e' || current_char == 'E'))
    {
        if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
        {
            cma_free(number);
            return (ft_nullptr);
        }
        json_stream_reader_consume(reader, &current_char);
        number[length] = current_char;
        length += 1;
        status = json_stream_reader_peek(reader, &current_char);
        if (status == JSON_STREAM_STATUS_OK
            && (current_char == '-' || current_char == '+'))
        {
            if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
            {
                cma_free(number);
                return (ft_nullptr);
            }
            json_stream_reader_consume(reader, &current_char);
            number[length] = current_char;
            length += 1;
            status = json_stream_reader_peek(reader, &current_char);
        }
        while (status == JSON_STREAM_STATUS_OK
            && ft_isdigit(static_cast<unsigned char>(current_char)))
        {
            if (json_stream_ensure_capacity(&number, &capacity, length + 2) != 0)
            {
                cma_free(number);
                return (ft_nullptr);
            }
            json_stream_reader_consume(reader, &current_char);
            number[length] = current_char;
            length += 1;
            status = json_stream_reader_peek(reader, &current_char);
        }
    }
    if (!has_digits)
    {
        cma_free(number);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (json_stream_ensure_capacity(&number, &capacity, length + 1) != 0)
    {
        cma_free(number);
        return (ft_nullptr);
    }
    number[length] = '\0';
    ft_errno = ER_SUCCESS;
    return (number);
}

static char *json_stream_parse_value(json_stream_reader *reader)
{
    int status = json_stream_skip_whitespace(reader);
    if (status != JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    char current_char;
    status = json_stream_reader_peek(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (current_char == '"')
        return (json_stream_parse_string(reader));
    if (current_char == 't')
    {
        if (json_stream_match_literal(reader, "true") != 0)
            return (ft_nullptr);
        char *value = cma_strdup("true");
        if (!value)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        ft_errno = ER_SUCCESS;
        return (value);
    }
    if (current_char == 'f')
    {
        if (json_stream_match_literal(reader, "false") != 0)
            return (ft_nullptr);
        char *value = cma_strdup("false");
        if (!value)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        ft_errno = ER_SUCCESS;
        return (value);
    }
    if (ft_isdigit(static_cast<unsigned char>(current_char))
        || current_char == '-' || current_char == '+')
        return (json_stream_parse_number(reader));
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (ft_nullptr);
}

static json_item *json_stream_parse_items(json_stream_reader *reader)
{
    if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (json_stream_reader_expect(reader, '{') != 0)
        return (ft_nullptr);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    bool object_closed = false;
    while (!object_closed)
    {
        int status = json_stream_skip_whitespace(reader);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        char current_char;
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (current_char == '}')
        {
            json_stream_reader_consume(reader, &current_char);
            object_closed = true;
            break;
        }
        char *key = json_stream_parse_string(reader);
        if (!key)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(key);
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (json_stream_reader_expect(reader, ':') != 0)
        {
            cma_free(key);
            json_free_items(head);
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(key);
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        char *value = json_stream_parse_value(reader);
        if (!value)
        {
            cma_free(key);
            json_free_items(head);
            return (ft_nullptr);
        }
        json_item *item = json_create_item(key, value);
        cma_free(value);
        cma_free(key);
        if (!item)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        if (!head)
        {
            head = item;
            tail = item;
        }
        else
        {
            tail->next = item;
            tail = item;
        }
        status = json_stream_skip_whitespace(reader);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (current_char == ',')
        {
            json_stream_reader_consume(reader, &current_char);
            continue;
        }
        if (current_char == '}')
            continue;
        json_free_items(head);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (!object_closed)
    {
        json_free_items(head);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (head);
}

static int json_stream_dispatch_event(json_stream_event_callback callback,
    void *user_data,
    json_stream_event_type type,
    char *string_value,
    bool bool_value)
{
    json_stream_event event;

    ft_bzero(&event, sizeof(event));
    event.type = type;
    event.bool_value = bool_value;
    if (string_value != ft_nullptr)
    {
        event.value.data = string_value;
        event.value.length = ft_strlen_size_t(string_value);
    }
    else
    {
        event.value.data = ft_nullptr;
        event.value.length = 0;
    }
    if (callback != ft_nullptr)
    {
        int callback_status;

        callback_status = callback(user_data, &event);
        if (string_value != ft_nullptr)
            cma_free(string_value);
        if (callback_status != 0)
        {
            ft_errno = FT_ERR_INVALID_OPERATION;
            return (-1);
        }
    }
    else if (string_value != ft_nullptr)
        cma_free(string_value);
    ft_errno = ER_SUCCESS;
    return (0);
}

static int json_stream_read_value(json_stream_reader *reader,
    json_stream_event_callback callback,
    void *user_data);

static int json_stream_read_array(json_stream_reader *reader,
    json_stream_event_callback callback,
    void *user_data)
{
    if (json_stream_reader_expect(reader, '[') != 0)
        return (-1);
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BEGIN_ARRAY,
            ft_nullptr,
            false) != 0)
        return (-1);
    bool finished = false;
    bool first_entry = true;
    while (!finished)
    {
        int status = json_stream_skip_whitespace(reader);
        if (status == JSON_STREAM_STATUS_ERROR)
            return (-1);
        if (status == JSON_STREAM_STATUS_OK)
        {
            char current_char;

            status = json_stream_reader_peek(reader, &current_char);
            if (status != JSON_STREAM_STATUS_OK)
                return (-1);
            if (current_char == ']')
            {
                json_stream_reader_consume(reader, &current_char);
                finished = true;
                break ;
            }
            if (!first_entry)
            {
                if (current_char != ',')
                {
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    return (-1);
                }
                json_stream_reader_consume(reader, &current_char);
                status = json_stream_skip_whitespace(reader);
                if (status != JSON_STREAM_STATUS_OK)
                    return (-1);
                status = json_stream_reader_peek(reader, &current_char);
                if (status != JSON_STREAM_STATUS_OK)
                    return (-1);
                if (current_char == ']')
                {
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    return (-1);
                }
            }
            if (json_stream_read_value(reader, callback, user_data) != 0)
                return (-1);
            first_entry = false;
            continue ;
        }
        if (status == JSON_STREAM_STATUS_END)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
    }
    if (!finished)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (json_stream_dispatch_event(callback,
        user_data,
        JSON_STREAM_EVENT_END_ARRAY,
        ft_nullptr,
        false));
}

static int json_stream_read_object(json_stream_reader *reader,
    json_stream_event_callback callback,
    void *user_data)
{
    if (json_stream_reader_expect(reader, '{') != 0)
        return (-1);
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BEGIN_OBJECT,
            ft_nullptr,
            false) != 0)
        return (-1);
    bool finished = false;
    bool first_entry = true;
    while (!finished)
    {
        int status = json_stream_skip_whitespace(reader);
        if (status == JSON_STREAM_STATUS_ERROR)
            return (-1);
        if (status == JSON_STREAM_STATUS_OK)
        {
            char current_char;

            status = json_stream_reader_peek(reader, &current_char);
            if (status != JSON_STREAM_STATUS_OK)
                return (-1);
            if (current_char == '}')
            {
                json_stream_reader_consume(reader, &current_char);
                finished = true;
                break ;
            }
            if (!first_entry)
            {
                if (current_char != ',')
                {
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    return (-1);
                }
                json_stream_reader_consume(reader, &current_char);
                status = json_stream_skip_whitespace(reader);
                if (status != JSON_STREAM_STATUS_OK)
                    return (-1);
                status = json_stream_reader_peek(reader, &current_char);
                if (status != JSON_STREAM_STATUS_OK)
                    return (-1);
                if (current_char == '}' || current_char == ',')
                {
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    return (-1);
                }
            }
            char *key = json_stream_parse_string(reader);
            if (!key)
                return (-1);
            if (json_stream_dispatch_event(callback,
                    user_data,
                    JSON_STREAM_EVENT_KEY,
                    key,
                    false) != 0)
                return (-1);
            status = json_stream_skip_whitespace(reader);
            if (status != JSON_STREAM_STATUS_OK)
                return (-1);
            if (json_stream_reader_expect(reader, ':') != 0)
                return (-1);
            if (json_stream_read_value(reader, callback, user_data) != 0)
                return (-1);
            first_entry = false;
            continue ;
        }
        if (status == JSON_STREAM_STATUS_END)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
    }
    if (!finished)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (json_stream_dispatch_event(callback,
        user_data,
        JSON_STREAM_EVENT_END_OBJECT,
        ft_nullptr,
        false));
}

static int json_stream_read_value(json_stream_reader *reader,
    json_stream_event_callback callback,
    void *user_data)
{
    int status = json_stream_skip_whitespace(reader);
    if (status != JSON_STREAM_STATUS_OK)
        return (-1);
    char current_char;

    status = json_stream_reader_peek(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
        return (-1);
    if (current_char == '"')
    {
        char *string_value = json_stream_parse_string(reader);
        if (!string_value)
            return (-1);
        return (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_STRING,
            string_value,
            false));
    }
    if (current_char == '{')
        return (json_stream_read_object(reader, callback, user_data));
    if (current_char == '[')
        return (json_stream_read_array(reader, callback, user_data));
    if (current_char == 't')
    {
        if (json_stream_match_literal(reader, "true") != 0)
            return (-1);
        return (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BOOLEAN,
            ft_nullptr,
            true));
    }
    if (current_char == 'f')
    {
        if (json_stream_match_literal(reader, "false") != 0)
            return (-1);
        return (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BOOLEAN,
            ft_nullptr,
            false));
    }
    if (current_char == 'n')
    {
        if (json_stream_match_literal(reader, "null") != 0)
            return (-1);
        return (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_NULL,
            ft_nullptr,
            false));
    }
    char *number_value = json_stream_parse_number(reader);
    if (!number_value)
        return (-1);
    return (json_stream_dispatch_event(callback,
        user_data,
        JSON_STREAM_EVENT_NUMBER,
        number_value,
        false));
}

int json_stream_reader_traverse(json_stream_reader *reader,
    json_stream_event_callback callback,
    void *user_data)
{
    if (!reader)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    reader->error_code = ER_SUCCESS;
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BEGIN_DOCUMENT,
            ft_nullptr,
            false) != 0)
    {
        reader->error_code = ft_errno;
        return (-1);
    }
    if (json_stream_read_value(reader, callback, user_data) != 0)
    {
        if (reader->error_code == ER_SUCCESS)
            reader->error_code = ft_errno;
        return (-1);
    }
    int status = json_stream_skip_whitespace(reader);
    if (status == JSON_STREAM_STATUS_ERROR)
    {
        if (reader->error_code == ER_SUCCESS)
            reader->error_code = ft_errno;
        return (-1);
    }
    if (status == JSON_STREAM_STATUS_OK)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_END_DOCUMENT,
            ft_nullptr,
            false) != 0)
    {
        if (reader->error_code == ER_SUCCESS)
            reader->error_code = ft_errno;
        return (-1);
    }
    reader->error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    return (0);
}

int json_stream_read_from_file_events(FILE *file,
    size_t buffer_capacity,
    json_stream_event_callback callback,
    void *user_data)
{
    if (!file || buffer_capacity == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_stream_reader reader;
    if (json_stream_reader_init_file(&reader, file, buffer_capacity) != 0)
        return (-1);
    int status = json_stream_reader_traverse(&reader, callback, user_data);
    int error_code = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (status != 0)
    {
        if (error_code != ER_SUCCESS)
            ft_errno = error_code;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int json_stream_read_from_stream_events(json_stream_read_callback callback,
    void *user_data,
    size_t buffer_capacity,
    json_stream_event_callback event_callback,
    void *event_user_data)
{
    if (!callback || buffer_capacity == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_stream_reader reader;
    if (json_stream_reader_init_callback(&reader, callback, user_data, buffer_capacity) != 0)
        return (-1);
    int status = json_stream_reader_traverse(&reader, event_callback, event_user_data);
    int error_code = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (status != 0)
    {
        if (error_code != ER_SUCCESS)
            ft_errno = error_code;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

json_group *json_stream_reader_parse(json_stream_reader *reader)
{
    if (!reader)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    reader->error_code = ER_SUCCESS;
    int status = json_stream_skip_whitespace(reader);
    if (status != JSON_STREAM_STATUS_OK)
    {
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (json_stream_reader_expect(reader, '{') != 0)
    {
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    bool object_closed = false;
    while (!object_closed)
    {
        status = json_stream_skip_whitespace(reader);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        char current_char;
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (current_char == '}')
        {
            json_stream_reader_consume(reader, &current_char);
            object_closed = true;
            break;
        }
        char *group_name = json_stream_parse_string(reader);
        if (!group_name)
        {
            json_free_groups(head);
            reader->error_code = ft_errno;
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (json_stream_reader_expect(reader, ':') != 0)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        json_item *items = json_stream_parse_items(reader);
        if (!items)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = ft_errno;
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        cma_free(group_name);
        if (!group)
        {
            json_free_items(items);
            json_free_groups(head);
            reader->error_code = ft_errno;
            return (ft_nullptr);
        }
        group->items = items;
        if (!head)
        {
            head = group;
            tail = group;
        }
        else
        {
            tail->next = group;
            tail = group;
        }
        status = json_stream_skip_whitespace(reader);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        if (current_char == ',')
        {
            json_stream_reader_consume(reader, &current_char);
            continue;
        }
        if (current_char == '}')
            continue;
        json_free_groups(head);
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (!object_closed)
    {
        json_free_groups(head);
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    reader->error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    return (head);
}

int json_stream_reader_init_callback(json_stream_reader *reader, json_stream_read_callback callback, void *user_data, size_t buffer_capacity)
{
    if (!reader || !callback || buffer_capacity == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_bzero(reader, sizeof(*reader));
    reader->read_callback = callback;
    reader->user_data = user_data;
    reader->buffer = static_cast<char *>(cma_malloc(buffer_capacity));
    if (!reader->buffer)
    {
        reader->error_code = FT_ERR_NO_MEMORY;
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    reader->buffer_capacity = buffer_capacity;
    reader->buffer_size = 0;
    reader->buffer_index = 0;
    reader->end_of_stream = false;
    reader->error_code = ER_SUCCESS;
    return (0);
}

int json_stream_reader_init_file(json_stream_reader *reader, FILE *file, size_t buffer_capacity)
{
    if (!file)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (json_stream_reader_init_callback(reader, json_stream_reader_file_callback, file, buffer_capacity));
}

void json_stream_reader_destroy(json_stream_reader *reader)
{
    if (!reader)
        return ;
    if (reader->buffer)
        cma_free(reader->buffer);
    reader->buffer = ft_nullptr;
    reader->buffer_capacity = 0;
    reader->buffer_size = 0;
    reader->buffer_index = 0;
    reader->end_of_stream = false;
    reader->error_code = ER_SUCCESS;
    reader->read_callback = ft_nullptr;
    reader->user_data = ft_nullptr;
    return ;
}

json_group *json_read_from_file_stream(FILE *file, size_t buffer_capacity)
{
    if (!file)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    json_stream_reader reader;
    if (json_stream_reader_init_file(&reader, file, buffer_capacity) != 0)
        return (ft_nullptr);
    json_group *groups = json_stream_reader_parse(&reader);
    int error_code = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (!groups)
    {
        if (error_code != ER_SUCCESS)
            ft_errno = error_code;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_INVALID_ARGUMENT;
    }
    return (groups);
}

json_group *json_read_from_stream(json_stream_read_callback callback, void *user_data, size_t buffer_capacity)
{
    if (!callback || buffer_capacity == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    json_stream_reader reader;
    if (json_stream_reader_init_callback(&reader, callback, user_data, buffer_capacity) != 0)
        return (ft_nullptr);
    json_group *groups = json_stream_reader_parse(&reader);
    int error_code = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (!groups)
    {
        if (error_code != ER_SUCCESS)
            ft_errno = error_code;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_INVALID_ARGUMENT;
    }
    return (groups);
}


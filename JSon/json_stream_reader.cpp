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

static void json_stream_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

static int json_stream_pop_error(void)
{
    return (ft_global_error_stack_drop_last_error());
}

#define JSON_STREAM_ERROR_RETURN(code, value) \
    do { json_stream_push_error(code); return (value); } while (0)

#define JSON_STREAM_SUCCESS_RETURN(value) \
    do { json_stream_push_error(FT_ERR_SUCCESSS); return (value); } while (0)

struct json_stream_reader_error_guard
{
    int *code;
    json_stream_reader_error_guard(int *value)
        : code(value)
    {
        return ;
    }
    ~json_stream_reader_error_guard()
    {
        ft_global_error_stack_push(*this->code);
        return ;
    }
};

static int json_stream_last_error(void)
{
    return (ft_global_error_stack_peek_last_error());
}

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
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, JSON_STREAM_STATUS_ERROR);
    if (reader->buffer_index < reader->buffer_size)
        JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_OK);
    if (reader->end_of_stream)
        JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_END);
    size_t bytes_read = reader->read_callback(reader->user_data, reader->buffer, reader->buffer_capacity);
    if (bytes_read == JSON_STREAM_READ_ERROR)
    {
        reader->error_code = FT_ERR_IO;
        JSON_STREAM_ERROR_RETURN(FT_ERR_IO, JSON_STREAM_STATUS_ERROR);
    }
    if (bytes_read == 0)
    {
        reader->end_of_stream = true;
        JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_END);
    }
    reader->buffer_index = 0;
    reader->buffer_size = bytes_read;
    JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_peek(json_stream_reader *reader, char *out_char)
{
    if (!reader || !out_char)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, JSON_STREAM_STATUS_ERROR);
    int status = json_stream_reader_fill(reader);
    int fill_error = json_stream_pop_error();
    if (fill_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(fill_error);
        return (status);
    }
    if (status != JSON_STREAM_STATUS_OK)
        JSON_STREAM_SUCCESS_RETURN(status);
    *out_char = reader->buffer[reader->buffer_index];
    JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_consume(json_stream_reader *reader, char *out_char)
{
    int status = json_stream_reader_peek(reader, out_char);
    int peek_error = json_stream_pop_error();
    if (peek_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(peek_error);
        return (status);
    }
    if (status != JSON_STREAM_STATUS_OK)
        JSON_STREAM_SUCCESS_RETURN(status);
    reader->buffer_index += 1;
    JSON_STREAM_SUCCESS_RETURN(JSON_STREAM_STATUS_OK);
}

static int json_stream_reader_expect(json_stream_reader *reader, char expected_char)
{
    char current_char;
    int status = json_stream_reader_consume(reader, &current_char);
    int consume_error = json_stream_pop_error();
    if (consume_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(consume_error);
        return (-1);
    }
    if (status != JSON_STREAM_STATUS_OK || current_char != expected_char)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    JSON_STREAM_SUCCESS_RETURN(0);
}

static int json_stream_skip_whitespace(json_stream_reader *reader)
{
    char current_char;
    int status = json_stream_reader_peek(reader, &current_char);
    int peek_error = json_stream_pop_error();
    if (peek_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(peek_error);
        return (status);
    }
    while (status == JSON_STREAM_STATUS_OK
        && ft_isspace(static_cast<unsigned char>(current_char)))
    {
        int consume_status = json_stream_reader_consume(reader, &current_char);
        int consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(consume_error);
            return (consume_status);
        }
        if (consume_status != JSON_STREAM_STATUS_OK)
            JSON_STREAM_SUCCESS_RETURN(consume_status);
        status = json_stream_reader_peek(reader, &current_char);
        peek_error = json_stream_pop_error();
        if (peek_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(peek_error);
            return (status);
        }
    }
    JSON_STREAM_SUCCESS_RETURN(status);
}

static int json_stream_ensure_capacity(char **buffer, size_t *capacity, size_t minimum)
{
    if (!buffer || !capacity)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    if (*capacity > minimum)
        JSON_STREAM_SUCCESS_RETURN(0);
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
    int realloc_error = json_stream_pop_error();
    if (!resized_buffer)
    {
        if (realloc_error == FT_ERR_SUCCESSS)
            realloc_error = FT_ERR_NO_MEMORY;
        JSON_STREAM_ERROR_RETURN(realloc_error, -1);
    }
    *buffer = resized_buffer;
    *capacity = new_capacity;
    JSON_STREAM_SUCCESS_RETURN(0);
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
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    if (code_point <= 0x7F)
    {
        int ensure_status = json_stream_ensure_capacity(buffer, capacity, *length + 1);
        int ensure_error = json_stream_pop_error();
        if (ensure_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(ensure_error);
            return (ensure_status);
        }
        (*buffer)[*length] = static_cast<char>(code_point);
        *length += 1;
        JSON_STREAM_SUCCESS_RETURN(0);
    }
    if (code_point <= 0x7FF)
    {
        int ensure_status = json_stream_ensure_capacity(buffer, capacity, *length + 2);
        int ensure_error = json_stream_pop_error();
        if (ensure_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(ensure_error);
            return (ensure_status);
        }
        (*buffer)[*length] = static_cast<char>(0xC0 | (code_point >> 6));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        JSON_STREAM_SUCCESS_RETURN(0);
    }
    if (code_point <= 0xFFFF)
    {
        int ensure_status = json_stream_ensure_capacity(buffer, capacity, *length + 3);
        int ensure_error = json_stream_pop_error();
        if (ensure_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(ensure_error);
            return (ensure_status);
        }
        (*buffer)[*length] = static_cast<char>(0xE0 | (code_point >> 12));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        JSON_STREAM_SUCCESS_RETURN(0);
    }
    if (code_point <= 0x10FFFF)
    {
        int ensure_status = json_stream_ensure_capacity(buffer, capacity, *length + 4);
        int ensure_error = json_stream_pop_error();
        if (ensure_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(ensure_error);
            return (ensure_status);
        }
        (*buffer)[*length] = static_cast<char>(0xF0 | (code_point >> 18));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        *length += 1;
        (*buffer)[*length] = static_cast<char>(0x80 | (code_point & 0x3F));
        *length += 1;
        JSON_STREAM_SUCCESS_RETURN(0);
    }
    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
}

static int json_stream_parse_code_unit(json_stream_reader *reader, unsigned int &code_unit)
{
    code_unit = 0;
    size_t digit_index = 0;
    while (digit_index < 4)
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        int consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(consume_error);
            return (-1);
        }
        if (status != JSON_STREAM_STATUS_OK)
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        unsigned int digit_value = 0;
        if (json_stream_hex_value(current_char, digit_value) != 0)
        {
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
            return (-1);
        }
        code_unit = (code_unit << 4) | digit_value;
        digit_index += 1;
    }
    JSON_STREAM_SUCCESS_RETURN(0);
}

static int json_stream_append_escape(json_stream_reader *reader,
    char **buffer,
    size_t *capacity,
    size_t *length)
{
    char escape_char;
    int status = json_stream_reader_consume(reader, &escape_char);
    int consume_error = json_stream_pop_error();
    if (consume_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(consume_error);
        return (status);
    }
    if (status != JSON_STREAM_STATUS_OK)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    unsigned int code_unit = 0;
    if (json_stream_parse_code_unit(reader, code_unit) != 0)
        return (-1);
    unsigned int code_point = code_unit;
    if (code_unit >= 0xD800 && code_unit <= 0xDBFF)
    {
        char next_char;
        status = json_stream_reader_peek(reader, &next_char);
        if (status != JSON_STREAM_STATUS_OK || next_char != '\\')
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        status = json_stream_reader_consume(reader, &next_char);
        consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(consume_error);
            return (status);
        }
        if (status != JSON_STREAM_STATUS_OK)
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        status = json_stream_reader_consume(reader, &next_char);
        consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(consume_error);
            return (status);
        }
        if (status != JSON_STREAM_STATUS_OK || next_char != 'u')
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        unsigned int low_unit = 0;
        if (json_stream_parse_code_unit(reader, low_unit) != 0)
            return (-1);
        if (low_unit < 0xDC00 || low_unit > 0xDFFF)
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        code_point = 0x10000;
        code_point = code_point + ((code_unit - 0xD800) << 10);
        code_point = code_point + (low_unit - 0xDC00);
    }
    else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
        JSON_STREAM_ERROR_RETURN(FT_ERR_NO_MEMORY, ft_nullptr);
    }
    bool closed = false;
    while (!closed)
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        int consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            cma_free(result);
            json_stream_push_error(consume_error);
            return (ft_nullptr);
        }
        if (status != JSON_STREAM_STATUS_OK)
        {
            cma_free(result);
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    }
    if (json_stream_ensure_capacity(&result, &capacity, length + 1) != 0)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    result[length] = '\0';
    JSON_STREAM_SUCCESS_RETURN(result);
}

static int json_stream_match_literal(json_stream_reader *reader, const char *literal)
{
    size_t index = 0;
    while (literal[index])
    {
        char current_char;
        int status = json_stream_reader_consume(reader, &current_char);
        int consume_error = json_stream_pop_error();
        if (consume_error != FT_ERR_SUCCESSS)
        {
            json_stream_push_error(consume_error);
            return (status);
        }
        if (status != JSON_STREAM_STATUS_OK)
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        if (current_char != literal[index])
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
        index += 1;
    }
    JSON_STREAM_SUCCESS_RETURN(0);
}

static char *json_stream_parse_number(json_stream_reader *reader)
{
    size_t capacity = 32;
    size_t length = 0;
    char *number = static_cast<char *>(cma_malloc(capacity));
    if (!number)
    {
        JSON_STREAM_ERROR_RETURN(FT_ERR_NO_MEMORY, ft_nullptr);
    }
    bool has_digits = false;
    char current_char;
    int status = json_stream_reader_peek(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
    {
        cma_free(number);
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    }
    if (json_stream_ensure_capacity(&number, &capacity, length + 1) != 0)
    {
        cma_free(number);
        return (ft_nullptr);
    }
    number[length] = '\0';
    JSON_STREAM_SUCCESS_RETURN(number);
}

static char *json_stream_parse_value(json_stream_reader *reader)
{
    int status = json_stream_skip_whitespace(reader);
    if (status != JSON_STREAM_STATUS_OK)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    char current_char;
    status = json_stream_reader_peek(reader, &current_char);
    if (status != JSON_STREAM_STATUS_OK)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    if (current_char == '"')
        return (json_stream_parse_string(reader));
    if (current_char == 't')
    {
        if (json_stream_match_literal(reader, "true") != 0)
            return (ft_nullptr);
        char *value = cma_strdup("true");
        if (!value)
        {
            JSON_STREAM_ERROR_RETURN(FT_ERR_NO_MEMORY, ft_nullptr);
        }
        JSON_STREAM_SUCCESS_RETURN(value);
    }
    if (current_char == 'f')
    {
        if (json_stream_match_literal(reader, "false") != 0)
            return (ft_nullptr);
        char *value = cma_strdup("false");
        if (!value)
        {
            JSON_STREAM_ERROR_RETURN(FT_ERR_NO_MEMORY, ft_nullptr);
        }
        JSON_STREAM_SUCCESS_RETURN(value);
    }
    if (ft_isdigit(static_cast<unsigned char>(current_char))
        || current_char == '-' || current_char == '+')
        return (json_stream_parse_number(reader));
    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
}

static json_item *json_stream_parse_items(json_stream_reader *reader)
{
    if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
        }
        char current_char;
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
        }
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_items(head);
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
        }
        if (current_char == ',')
        {
            json_stream_reader_consume(reader, &current_char);
            continue;
        }
        if (current_char == '}')
            continue;
        json_free_items(head);
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    }
    if (!object_closed)
    {
        json_free_items(head);
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    }
    JSON_STREAM_SUCCESS_RETURN(head);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_OPERATION, -1);
        }
    }
    else if (string_value != ft_nullptr)
        cma_free(string_value);
    JSON_STREAM_SUCCESS_RETURN(0);
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
                    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
                    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
                }
            }
            if (json_stream_read_value(reader, callback, user_data) != 0)
                return (-1);
            first_entry = false;
            continue ;
        }
        if (status == JSON_STREAM_STATUS_END)
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    }
    if (!finished)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
                    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
                    JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
            JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    }
    if (!finished)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
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
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int status;
    int error_code = FT_ERR_SUCCESSS;
    json_stream_reader_error_guard stack_guard(&error_code);

    if (!reader)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (json_stream_reader_enable_thread_safety(reader) != 0)
        return (-1);
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        error_code = lock_error;
        return (-1);
    }
    reader->error_code = FT_ERR_SUCCESSS;
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_BEGIN_DOCUMENT,
            ft_nullptr,
            false) != 0)
    {
        reader->error_code = error_code = json_stream_last_error();
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    if (json_stream_read_value(reader, callback, user_data) != 0)
    {
        if (reader->error_code == FT_ERR_SUCCESSS)
            reader->error_code = error_code = json_stream_last_error();
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    status = json_stream_skip_whitespace(reader);
    if (status == JSON_STREAM_STATUS_ERROR)
    {
        if (reader->error_code == FT_ERR_SUCCESSS)
            reader->error_code = error_code = json_stream_last_error();
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    if (status == JSON_STREAM_STATUS_OK)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    if (json_stream_dispatch_event(callback,
            user_data,
            JSON_STREAM_EVENT_END_DOCUMENT,
            ft_nullptr,
            false) != 0)
    {
        if (reader->error_code == FT_ERR_SUCCESSS)
            reader->error_code = error_code = json_stream_last_error();
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    reader->error_code = FT_ERR_SUCCESSS;
    error_code = FT_ERR_SUCCESSS;
    json_stream_reader_finalize_lock(reader, guard);
    return (0);
}

int json_stream_read_from_file_events(FILE *file,
    size_t buffer_capacity,
    json_stream_event_callback callback,
    void *user_data)
{
    int error_code = FT_ERR_SUCCESSS;
    json_stream_reader_error_guard stack_guard(&error_code);
    if (!file || buffer_capacity == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_stream_reader reader = json_stream_reader();
    if (json_stream_reader_init_file(&reader, file, buffer_capacity) != 0)
        return (-1);
    int status = json_stream_reader_traverse(&reader, callback, user_data);
    int reader_error = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (status != 0)
    {
        if (reader_error != FT_ERR_SUCCESSS)
            error_code = reader_error;
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    error_code = FT_ERR_SUCCESSS;
    return (0);
}

int json_stream_read_from_stream_events(json_stream_read_callback callback,
    void *user_data,
    size_t buffer_capacity,
    json_stream_event_callback event_callback,
    void *event_user_data)
{
    int error_code = FT_ERR_SUCCESSS;
    json_stream_reader_error_guard stack_guard(&error_code);
    if (!callback || buffer_capacity == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_stream_reader reader = json_stream_reader();
    if (json_stream_reader_init_callback(&reader, callback, user_data, buffer_capacity) != 0)
        return (-1);
    int status = json_stream_reader_traverse(&reader, event_callback, event_user_data);
    int reader_error = reader.error_code;
    json_stream_reader_destroy(&reader);
    if (status != 0)
    {
        if (reader_error != FT_ERR_SUCCESSS)
            error_code = reader_error;
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    error_code = FT_ERR_SUCCESSS;
    return (0);
}

json_group *json_stream_reader_parse(json_stream_reader *reader)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int status;
    json_group *head;
    json_group *tail;
    bool object_closed;
    int error_code = FT_ERR_SUCCESSS;
    json_stream_reader_error_guard stack_guard(&error_code);

    if (!reader)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (json_stream_reader_enable_thread_safety(reader) != 0)
        return (ft_nullptr);
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        error_code = lock_error;
        return (ft_nullptr);
    }
    reader->error_code = FT_ERR_SUCCESSS;
    status = json_stream_skip_whitespace(reader);
    if (status != JSON_STREAM_STATUS_OK)
    {
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        error_code = FT_ERR_INVALID_ARGUMENT;
        json_stream_reader_finalize_lock(reader, guard);
        return (ft_nullptr);
    }
    if (json_stream_reader_expect(reader, '{') != 0)
    {
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        json_stream_reader_finalize_lock(reader, guard);
        return (ft_nullptr);
    }
    head = ft_nullptr;
    tail = ft_nullptr;
    object_closed = false;
    while (!object_closed)
    {
        status = json_stream_skip_whitespace(reader);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        char current_char;
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
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
            error_code = json_stream_last_error();
            reader->error_code = error_code;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        if (json_stream_reader_expect(reader, ':') != 0)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        if (json_stream_skip_whitespace(reader) != JSON_STREAM_STATUS_OK)
        {
            cma_free(group_name);
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_push_error(FT_ERR_INVALID_ARGUMENT);
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        json_item *items = json_stream_parse_items(reader);
        if (!items)
        {
            cma_free(group_name);
            json_free_groups(head);
            error_code = json_stream_last_error();
            reader->error_code = error_code;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        cma_free(group_name);
        if (!group)
        {
            json_free_items(items);
            json_free_groups(head);
            error_code = json_stream_last_error();
            reader->error_code = error_code;
            json_stream_reader_finalize_lock(reader, guard);
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
            error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
            return (ft_nullptr);
        }
        status = json_stream_reader_peek(reader, &current_char);
        if (status != JSON_STREAM_STATUS_OK)
        {
            json_free_groups(head);
            reader->error_code = FT_ERR_INVALID_ARGUMENT;
            error_code = FT_ERR_INVALID_ARGUMENT;
            json_stream_reader_finalize_lock(reader, guard);
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
        error_code = FT_ERR_INVALID_ARGUMENT;
        json_stream_reader_finalize_lock(reader, guard);
        return (ft_nullptr);
    }
    if (!object_closed)
    {
        json_free_groups(head);
        reader->error_code = FT_ERR_INVALID_ARGUMENT;
        error_code = FT_ERR_INVALID_ARGUMENT;
        json_stream_reader_finalize_lock(reader, guard);
        return (ft_nullptr);
    }
    reader->error_code = FT_ERR_SUCCESSS;
    json_stream_push_error(FT_ERR_SUCCESSS);
    json_stream_reader_finalize_lock(reader, guard);
    return (head);
}

int json_stream_reader_init_callback(json_stream_reader *reader, json_stream_read_callback callback, void *user_data, size_t buffer_capacity)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    char *new_buffer;

    if (!reader || !callback || buffer_capacity == 0)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    if (json_stream_reader_enable_thread_safety(reader) != 0)
        return (-1);
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(lock_error);
        return (-1);
    }
    new_buffer = static_cast<char *>(cma_malloc(buffer_capacity));
    if (!new_buffer)
    {
        reader->error_code = FT_ERR_NO_MEMORY;
        json_stream_reader_set_error_unlocked(reader, FT_ERR_NO_MEMORY);
        json_stream_push_error(FT_ERR_NO_MEMORY);
        json_stream_reader_finalize_lock(reader, guard);
        return (-1);
    }
    if (reader->buffer)
        cma_free(reader->buffer);
    reader->read_callback = callback;
    reader->user_data = user_data;
    reader->buffer = new_buffer;
    reader->buffer_capacity = buffer_capacity;
    reader->buffer_size = 0;
    reader->buffer_index = 0;
    reader->end_of_stream = false;
    reader->error_code = FT_ERR_SUCCESSS;
    json_stream_reader_set_error_unlocked(reader, FT_ERR_SUCCESSS);
    json_stream_reader_finalize_lock(reader, guard);
    return (0);
}

int json_stream_reader_init_file(json_stream_reader *reader, FILE *file, size_t buffer_capacity)
{
    if (!file)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    return (json_stream_reader_init_callback(reader, json_stream_reader_file_callback, file, buffer_capacity));
}

void json_stream_reader_destroy(json_stream_reader *reader)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (!reader)
        return ;
    if (json_stream_reader_enable_thread_safety(reader) != 0)
        return ;
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        json_stream_push_error(lock_error);
        return ;
    }
    if (reader->buffer)
        cma_free(reader->buffer);
    reader->buffer = ft_nullptr;
    reader->buffer_capacity = 0;
    reader->buffer_size = 0;
    reader->buffer_index = 0;
    reader->end_of_stream = false;
    reader->error_code = FT_ERR_SUCCESSS;
    reader->read_callback = ft_nullptr;
    reader->user_data = ft_nullptr;
    json_stream_reader_set_error_unlocked(reader, FT_ERR_SUCCESSS);
    json_stream_reader_finalize_lock(reader, guard);
    return ;
}

json_group *json_read_from_file_stream(FILE *file, size_t buffer_capacity)
{
    if (!file)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    json_group *groups;
    int result_errno;

    groups = ft_nullptr;
    result_errno = FT_ERR_SUCCESSS;
    {
        json_stream_reader reader = json_stream_reader();

        if (json_stream_reader_init_file(&reader, file, buffer_capacity) != 0)
            return (ft_nullptr);
        groups = json_stream_reader_parse(&reader);
        if (!groups)
        {
            int parse_errno;
            int error_code;

            parse_errno = json_stream_last_error();
            error_code = reader.error_code;
            result_errno = error_code;
            if (result_errno == FT_ERR_SUCCESSS)
                result_errno = parse_errno;
            if (result_errno == FT_ERR_SUCCESSS)
                result_errno = FT_ERR_INVALID_ARGUMENT;
        }
        json_stream_reader_destroy(&reader);
    }
        if (!groups)
            json_stream_push_error(result_errno);
    return (groups);
}

json_group *json_read_from_stream(json_stream_read_callback callback, void *user_data, size_t buffer_capacity)
{
    if (!callback || buffer_capacity == 0)
        JSON_STREAM_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    json_group *groups;
    int result_errno;

    groups = ft_nullptr;
    result_errno = FT_ERR_SUCCESSS;
    {
        json_stream_reader reader = json_stream_reader();

        if (json_stream_reader_init_callback(&reader, callback, user_data, buffer_capacity) != 0)
            return (ft_nullptr);
        groups = json_stream_reader_parse(&reader);
        if (!groups)
        {
            int parse_errno;
            int error_code;

            parse_errno = json_stream_last_error();
            error_code = reader.error_code;
            result_errno = error_code;
            if (result_errno == FT_ERR_SUCCESSS)
                result_errno = parse_errno;
            if (result_errno == FT_ERR_SUCCESSS)
                result_errno = FT_ERR_INVALID_ARGUMENT;
        }
        json_stream_reader_destroy(&reader);
    }
        if (!groups)
            json_stream_push_error(result_errno);
    return (groups);
}

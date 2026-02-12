#include "../Basic/basic.hpp"
#include "json.hpp"
#include "document.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstdio>
#include <limits>

static void skip_whitespace(const char *json_string, size_t &index)
{
    while (json_string[index]
        && ft_isspace(static_cast<unsigned char>(json_string[index])))
        index++;
    return ;
}

static void json_reader_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

static int json_reader_pop_error(void)
{
    return (ft_global_error_stack_drop_last_error());
}

static void json_reader_set_io_error(int last_error)
{
    int normalized_error;

    normalized_error = cmp_decode_errno_offset_error(last_error);
    if (normalized_error == FT_ERR_SUCCESS)
        normalized_error = FT_ERR_IO;
    json_reader_push_error(normalized_error);
    return ;
}

static char *json_read_file_content(const char *filename)
{
    su_file *file_stream;
    long file_size_long;
    size_t file_size;
    char *content;
    size_t read_count;
    int io_error;

    file_stream = su_fopen(filename);
    io_error = json_reader_pop_error();
    if (file_stream == ft_nullptr)
    {
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    if (su_fseek(file_stream, 0, SEEK_END) != 0)
    {
        io_error = json_reader_pop_error();
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    json_reader_pop_error();
    file_size_long = su_ftell(file_stream);
    io_error = json_reader_pop_error();
    if (file_size_long < 0)
    {
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    if (su_fseek(file_stream, 0, SEEK_SET) != 0)
    {
        io_error = json_reader_pop_error();
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    json_reader_pop_error();
    if (static_cast<unsigned long long>(file_size_long)
        > static_cast<unsigned long long>(std::numeric_limits<size_t>::max()))
    {
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    file_size = static_cast<size_t>(file_size_long);
    content = static_cast<char *>(cma_malloc(file_size + 1));
    io_error = json_reader_pop_error();
    if (content == ft_nullptr)
    {
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_push_error(io_error);
        return (ft_nullptr);
    }
    read_count = su_fread(content, 1, file_size, file_stream);
    io_error = json_reader_pop_error();
    if (read_count != file_size)
    {
        cma_free(content);
        su_fclose(file_stream);
        json_reader_pop_error();
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    content[file_size] = '\0';
    if (su_fclose(file_stream) != 0)
    {
        io_error = json_reader_pop_error();
        cma_free(content);
        json_reader_set_io_error(io_error);
        return (ft_nullptr);
    }
    json_reader_pop_error();
    json_reader_push_error(FT_ERR_SUCCESS);
    return (content);
}

static int json_reader_hex_value(char character, unsigned int &value) noexcept
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

static int json_reader_ensure_capacity(char **buffer_ptr, size_t &capacity, size_t required) noexcept
{
    if (buffer_ptr == ft_nullptr)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (capacity > required)
        return (0);
    size_t new_capacity = capacity;
    if (new_capacity == 0)
        new_capacity = 16;
    while (new_capacity <= required)
    {
        size_t next_capacity = new_capacity * 2;
        if (next_capacity <= new_capacity)
        {
            json_reader_push_error(FT_ERR_OUT_OF_RANGE);
            return (-1);
        }
        new_capacity = next_capacity;
    }
    char *resized_buffer = static_cast<char *>(cma_realloc(*buffer_ptr, new_capacity));
    int realloc_error = json_reader_pop_error();
    if (!resized_buffer)
    {
        if (realloc_error == FT_ERR_SUCCESS)
            realloc_error = FT_ERR_NO_MEMORY;
        json_reader_push_error(realloc_error);
        return (-1);
    }
    *buffer_ptr = resized_buffer;
    capacity = new_capacity;
    json_reader_push_error(FT_ERR_SUCCESS);
    return (0);
}

static int json_reader_append_utf8(char **buffer_ptr,
    size_t &capacity,
    size_t &length,
    unsigned int code_point) noexcept
{
    if (code_point <= 0x7F)
    {
        if (json_reader_ensure_capacity(buffer_ptr, capacity, length + 1) != 0)
        {
            int capacity_error = json_reader_pop_error();
            json_reader_push_error(capacity_error);
            return (-1);
        }
        json_reader_pop_error();
        (*buffer_ptr)[length] = static_cast<char>(code_point);
        length++;
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (code_point <= 0x7FF)
    {
        if (json_reader_ensure_capacity(buffer_ptr, capacity, length + 2) != 0)
        {
            int capacity_error = json_reader_pop_error();
            json_reader_push_error(capacity_error);
            return (-1);
        }
        json_reader_pop_error();
        (*buffer_ptr)[length] = static_cast<char>(0xC0 | (code_point >> 6));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | (code_point & 0x3F));
        length++;
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (code_point <= 0xFFFF)
    {
        if (json_reader_ensure_capacity(buffer_ptr, capacity, length + 3) != 0)
        {
            int capacity_error = json_reader_pop_error();
            json_reader_push_error(capacity_error);
            return (-1);
        }
        json_reader_pop_error();
        (*buffer_ptr)[length] = static_cast<char>(0xE0 | (code_point >> 12));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | (code_point & 0x3F));
        length++;
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (code_point <= 0x10FFFF)
    {
        if (json_reader_ensure_capacity(buffer_ptr, capacity, length + 4) != 0)
        {
            int capacity_error = json_reader_pop_error();
            json_reader_push_error(capacity_error);
            return (-1);
        }
        json_reader_pop_error();
        (*buffer_ptr)[length] = static_cast<char>(0xF0 | (code_point >> 18));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        length++;
        (*buffer_ptr)[length] = static_cast<char>(0x80 | (code_point & 0x3F));
        length++;
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

static int json_reader_parse_code_unit(const char *json_string,
    size_t length,
    size_t &index,
    unsigned int &code_unit) noexcept
{
    code_unit = 0;
    size_t digit_index = 0;
    while (digit_index < 4)
    {
        if (index >= length)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        unsigned int digit_value = 0;
        if (json_reader_hex_value(json_string[index], digit_value) != 0)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        code_unit = (code_unit << 4) | digit_value;
        index++;
        digit_index++;
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (0);
}

static int json_reader_append_escape(const char *json_string,
    size_t length,
    size_t &index,
    char **buffer_ptr,
    size_t &capacity,
    size_t &out_length) noexcept
{
    if (index >= length)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    char escape_char = json_string[index];
    index++;
    if (escape_char == '"' || escape_char == '\\' || escape_char == '/')
    {
        if (json_reader_ensure_capacity(buffer_ptr, capacity, out_length + 1) != 0)
        {
            int capacity_error = json_reader_pop_error();
            json_reader_push_error(capacity_error);
            return (-1);
        }
        json_reader_pop_error();
        (*buffer_ptr)[out_length] = escape_char;
        out_length++;
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char == 'b')
    {
        int utf8_error = json_reader_append_utf8(buffer_ptr, capacity, out_length, '\b');
        if (utf8_error != 0)
        {
            int last_error = json_reader_pop_error();
            json_reader_push_error(last_error);
            return (-1);
        }
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char == 'f')
    {
        int utf8_error = json_reader_append_utf8(buffer_ptr, capacity, out_length, '\f');
        if (utf8_error != 0)
        {
            int last_error = json_reader_pop_error();
            json_reader_push_error(last_error);
            return (-1);
        }
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char == 'n')
    {
        int utf8_error = json_reader_append_utf8(buffer_ptr, capacity, out_length, '\n');
        if (utf8_error != 0)
        {
            int last_error = json_reader_pop_error();
            json_reader_push_error(last_error);
            return (-1);
        }
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char == 'r')
    {
        int utf8_error = json_reader_append_utf8(buffer_ptr, capacity, out_length, '\r');
        if (utf8_error != 0)
        {
            int last_error = json_reader_pop_error();
            json_reader_push_error(last_error);
            return (-1);
        }
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char == 't')
    {
        int utf8_error = json_reader_append_utf8(buffer_ptr, capacity, out_length, '\t');
        if (utf8_error != 0)
        {
            int last_error = json_reader_pop_error();
            json_reader_push_error(last_error);
            return (-1);
        }
        json_reader_pop_error();
        json_reader_push_error(FT_ERR_SUCCESS);
        return (0);
    }
    if (escape_char != 'u')
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    unsigned int code_unit = 0;
    if (json_reader_parse_code_unit(json_string, length, index, code_unit) != 0)
    {
        int parse_error = json_reader_pop_error();
        json_reader_push_error(parse_error);
        return (-1);
    }
    json_reader_pop_error();
    unsigned int code_point = code_unit;
    if (code_unit >= 0xD800 && code_unit <= 0xDBFF)
    {
        if (index + 1 >= length)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        if (json_string[index] != '\\' || json_string[index + 1] != 'u')
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        index = index + 2;
        unsigned int low_unit = 0;
        if (json_reader_parse_code_unit(json_string, length, index, low_unit) != 0)
        {
            int parse_error = json_reader_pop_error();
            json_reader_push_error(parse_error);
            return (-1);
        }
        json_reader_pop_error();
        if (low_unit < 0xDC00 || low_unit > 0xDFFF)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        code_point = 0x10000;
        code_point = code_point + ((code_unit - 0xD800) << 10);
        code_point = code_point + (low_unit - 0xDC00);
    }
    else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int utf8_result = json_reader_append_utf8(buffer_ptr, capacity, out_length, code_point);
    int utf8_error = json_reader_pop_error();
    if (utf8_result != 0)
    {
        json_reader_push_error(utf8_error);
        return (-1);
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (0);
}

static char *parse_string(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length || json_string[index] != '"')
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    index++;
    size_t capacity = 32;
    size_t out_length = 0;
    char *result = static_cast<char *>(cma_malloc(capacity));
    int allocation_error = json_reader_pop_error();
    if (!result)
    {
        json_reader_push_error(allocation_error);
        return (ft_nullptr);
    }
    bool closed = false;
    while (index < length)
    {
        char current_char = json_string[index];
        index++;
        if (current_char == '"')
        {
            closed = true;
            break;
        }
        if (current_char == '\\')
        {
            if (json_reader_append_escape(json_string,
                    length,
                    index,
                    &result,
                    capacity,
                    out_length) != 0)
            {
                int escape_error = json_reader_pop_error();
                cma_free(result);
                json_reader_push_error(escape_error);
                return (ft_nullptr);
            }
            json_reader_pop_error();
            continue;
        }
        if (static_cast<unsigned char>(current_char) < 0x20)
        {
            cma_free(result);
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        if (json_reader_ensure_capacity(&result, capacity, out_length + 1) != 0)
        {
            int capacity_error = json_reader_pop_error();
            cma_free(result);
            json_reader_push_error(capacity_error);
            return (ft_nullptr);
        }
        json_reader_pop_error();
        result[out_length] = current_char;
        out_length++;
    }
    if (!closed)
    {
        cma_free(result);
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (json_reader_ensure_capacity(&result, capacity, out_length + 1) != 0)
    {
        int capacity_error = json_reader_pop_error();
        cma_free(result);
        json_reader_push_error(capacity_error);
        return (ft_nullptr);
    }
    json_reader_pop_error();
    result[out_length] = '\0';
    json_reader_push_error(FT_ERR_SUCCESS);
    return (result);
}

static char *parse_number(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    size_t start_index = index;
    if (index < length && (json_string[index] == '-' || json_string[index] == '+'))
        index++;
    bool has_digits = false;
    while (index < length
        && ft_isdigit(static_cast<unsigned char>(json_string[index])))
    {
        index++;
        has_digits = true;
    }
    if (index < length && json_string[index] == '.')
    {
        size_t fractional_count;

        index++;
        fractional_count = 0;
        while (index < length
            && ft_isdigit(static_cast<unsigned char>(json_string[index])))
        {
            index++;
            fractional_count++;
        }
        if (fractional_count == 0)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
    }
    if (index < length && (json_string[index] == 'e' || json_string[index] == 'E'))
    {
        size_t exponent_count;

        index++;
        if (index < length && (json_string[index] == '-' || json_string[index] == '+'))
            index++;
        exponent_count = 0;
        while (index < length
            && ft_isdigit(static_cast<unsigned char>(json_string[index])))
        {
            index++;
            exponent_count++;
        }
        if (exponent_count == 0)
        {
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
    }
    if (!has_digits)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char *number = cma_substr(json_string,
                               static_cast<unsigned int>(start_index),
                               index - start_index);
    int substr_error = json_reader_pop_error();
    if (!number)
    {
        json_reader_push_error(substr_error);
        return (ft_nullptr);
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (number);
}

static char *parse_value(const char *json_string, size_t &index)
{
    skip_whitespace(json_string, index);
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (json_string[index] == '"')
    {
        char *value = parse_string(json_string, index);
        int string_error = json_reader_pop_error();
        if (!value)
        {
            json_reader_push_error(string_error);
            return (ft_nullptr);
        }
        json_reader_push_error(FT_ERR_SUCCESS);
        return (value);
    }
    if (length - index >= 4 && ft_strncmp(json_string + index, "true", 4) == 0)
    {
        index += 4;
        char *value = cma_strdup("true");
        int string_error = json_reader_pop_error();
        if (!value)
        {
            json_reader_push_error(string_error);
            return (ft_nullptr);
        }
        json_reader_push_error(FT_ERR_SUCCESS);
        return (value);
    }
    if (length - index >= 5 && ft_strncmp(json_string + index, "false", 5) == 0)
    {
        index += 5;
        char *value = cma_strdup("false");
        int string_error = json_reader_pop_error();
        if (!value)
        {
            json_reader_push_error(string_error);
            return (ft_nullptr);
        }
        json_reader_push_error(FT_ERR_SUCCESS);
        return (value);
    }
    if (ft_isdigit(static_cast<unsigned char>(json_string[index]))
        || json_string[index] == '-' || json_string[index] == '+')
    {
        char *value = parse_number(json_string, index);
        int number_error = json_reader_pop_error();
        if (!value)
        {
            json_reader_push_error(number_error);
            return (ft_nullptr);
        }
        json_reader_push_error(FT_ERR_SUCCESS);
        return (value);
    }
    json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
    return (ft_nullptr);
}

static json_item *parse_items(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_whitespace(json_string, index);
    if (index >= length || json_string[index] != '{')
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    index++;
    bool object_closed = false;
    while (index < length)
    {
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == '}')
        {
            index++;
            object_closed = true;
            break;
        }
        char *key = parse_string(json_string, index);
        int key_error = json_reader_pop_error();
        if (!key)
        {
            json_free_items(head);
            json_reader_push_error(key_error);
            return (ft_nullptr);
        }
        skip_whitespace(json_string, index);
        if (index >= length || json_string[index] != ':')
        {
            cma_free(key);
            json_free_items(head);
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        index++;
        skip_whitespace(json_string, index);
        char *value = parse_value(json_string, index);
        int value_error = json_reader_pop_error();
        if (!value)
        {
            cma_free(key);
            json_free_items(head);
            json_reader_push_error(value_error);
            return (ft_nullptr);
        }
        json_item *item = json_create_item(key, value);
        int item_error = json_reader_pop_error();
        cma_free(value);
        cma_free(key);
        if (!item)
        {
            json_free_items(head);
            json_reader_push_error(item_error);
            return (ft_nullptr);
        }
        if (!head)
            head = tail = item;
        else
        {
            tail->next = item;
            tail = item;
        }
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == ',')
        {
            index++;
            continue;
        }
    }
    if (!object_closed)
    {
        json_free_items(head);
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (head);
}

json_group *json_read_from_file(const char *filename)
{
    char *content;
    size_t index;
    size_t length;
    json_group *head;
    json_group *tail;
    bool object_closed;

    content = json_read_file_content(filename);
    int content_error = json_reader_pop_error();
    if (content == ft_nullptr)
    {
        json_reader_push_error(content_error);
        return (ft_nullptr);
    }
    index = 0;
    skip_whitespace(content, index);
    length = ft_strlen_size_t(content);
    if (index >= length || content[index] != '{')
    {
        cma_free(content);
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    index++;
    head = ft_nullptr;
    tail = ft_nullptr;
    object_closed = false;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            object_closed = true;
            break;
        }
        char *group_name = parse_string(content, index);
        int name_error = json_reader_pop_error();
        if (!group_name)
        {
            json_free_groups(head);
            cma_free(content);
            json_reader_push_error(name_error);
            return (ft_nullptr);
        }
        skip_whitespace(content, index);
        if (index >= length || content[index] != ':')
        {
            cma_free(group_name);
            json_free_groups(head);
            cma_free(content);
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        index++;
        json_item *items = parse_items(content, index);
        int items_error = json_reader_pop_error();
        if (!items)
        {
            cma_free(group_name);
            json_free_groups(head);
            cma_free(content);
            json_reader_push_error(items_error);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        int group_error = json_reader_pop_error();
        cma_free(group_name);
        if (!group)
        {
            json_free_items(items);
            json_free_groups(head);
            cma_free(content);
            json_reader_push_error(group_error);
            return (ft_nullptr);
        }
        group->items = items;
        if (!head)
            head = tail = group;
        else
        {
            tail->next = group;
            tail = group;
        }
        skip_whitespace(content, index);
        if (index < length && content[index] == ',')
        {
            index++;
            continue;
        }
    }
    if (!object_closed)
    {
        json_free_groups(head);
        cma_free(content);
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    cma_free(content);
    json_reader_push_error(FT_ERR_SUCCESS);
    return (head);
}

json_group *json_read_from_backend(ft_document_source &source)
{
    ft_string content_buffer;
    int read_result;

    read_result = source.read_all(content_buffer);
    if (read_result != FT_ERR_SUCCESS)
    {
        json_reader_push_error(read_result);
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(content_buffer.c_str());
    int parse_error = json_reader_pop_error();
    if (!result)
    {
        json_reader_push_error(parse_error);
        return (ft_nullptr);
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (result);
}

int json_document_read_from_backend(json_document &document, ft_document_source &source)
{
    return (document.read_from_backend(source));
}

json_group *json_read_from_string(const char *content)
{
    if (!content)
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    size_t index = 0;
    skip_whitespace(content, index);
    size_t length = ft_strlen_size_t(content);
    if (index >= length || content[index] != '{')
    {
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    index++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    bool object_closed = false;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            object_closed = true;
            break;
        }
        char *group_name = parse_string(content, index);
        int name_error = json_reader_pop_error();
        if (!group_name)
        {
            if (name_error == FT_ERR_SUCCESS)
                name_error = FT_ERR_INVALID_ARGUMENT;
            json_free_groups(head);
            json_reader_push_error(name_error);
            return (ft_nullptr);
        }
        skip_whitespace(content, index);
        if (index >= length || content[index] != ':')
        {
            cma_free(group_name);
            json_free_groups(head);
            json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        index++;
        json_item *items = parse_items(content, index);
        int items_error = json_reader_pop_error();
        if (!items)
        {
            if (items_error == FT_ERR_SUCCESS)
                items_error = FT_ERR_INVALID_ARGUMENT;
            cma_free(group_name);
            json_free_groups(head);
            json_reader_push_error(items_error);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        int group_error = json_reader_pop_error();
        cma_free(group_name);
        if (!group)
        {
            if (group_error == FT_ERR_SUCCESS)
                group_error = FT_ERR_INVALID_ARGUMENT;
            json_free_items(items);
            json_free_groups(head);
            json_reader_push_error(group_error);
            return (ft_nullptr);
        }
        group->items = items;
        if (!head)
            head = tail = group;
        else
        {
            tail->next = group;
            tail = group;
        }
        skip_whitespace(content, index);
        if (index < length && content[index] == ',')
        {
            index++;
            continue;
        }
    }
    if (!object_closed)
    {
        json_free_groups(head);
        json_reader_push_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_reader_push_error(FT_ERR_SUCCESS);
    return (head);
}

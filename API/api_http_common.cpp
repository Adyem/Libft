#include "api_http_common.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_big_number.hpp"
#include <cstddef>
#include <climits>

char api_http_to_lower(char character)
{
    if (character >= 'A' && character <= 'Z')
        character = static_cast<char>(character - 'A' + 'a');
    return (character);
}

bool api_http_is_whitespace(char character)
{
    if (character == ' ')
        return (true);
    if (character == '\t')
        return (true);
    if (character == '\r')
        return (true);
    if (character == '\n')
        return (true);
    return (false);
}

bool api_http_line_starts_with(const char *line_start, size_t line_length,
    const char *name)
{
    size_t name_length;
    size_t index;

    if (!line_start || !name)
        return (false);
    name_length = ft_strlen(name);
    if (line_length < name_length)
        return (false);
    index = 0;
    while (index < name_length)
    {
        char line_character;
        char name_character;

        line_character = api_http_to_lower(line_start[index]);
        name_character = api_http_to_lower(name[index]);
        if (line_character != name_character)
            return (false);
        index++;
    }
    if (line_length > name_length)
    {
        if (line_start[name_length] != ':')
            return (false);
    }
    return (true);
}

const char *api_http_find_crlf(const char *start, const char *end)
{
    const char *walker;

    if (!start || !end)
        return (ft_nullptr);
    walker = start;
    while (walker < end)
    {
        if (*walker == '\r')
        {
            if ((walker + 1) >= end)
                return (ft_nullptr);
            if (*(walker + 1) == '\n')
                return (walker);
        }
        walker++;
    }
    return (ft_nullptr);
}

void api_http_trim_header_value(const char **value_start,
    const char **value_end)
{
    const char *begin;
    const char *finish;

    if (!value_start || !value_end)
        return ;
    begin = *value_start;
    finish = *value_end;
    while (begin < finish && api_http_is_whitespace(*begin))
        begin++;
    while (finish > begin && api_http_is_whitespace(*(finish - 1)))
        finish--;
    *value_start = begin;
    *value_end = finish;
    return ;
}

bool api_http_parse_decimal(const char *start, const char *end,
    long long &value)
{
    const char *walker;
    long long result_value;

    if (!start || !end)
        return (false);
    walker = start;
    if (walker == end)
        return (false);
    result_value = 0;
    while (walker < end)
    {
        if (*walker < '0' || *walker > '9')
            return (false);
        if (result_value > (LLONG_MAX / 10))
            return (false);
        result_value *= 10;
        if (result_value > (LLONG_MAX - (*walker - '0')))
            return (false);
        result_value += static_cast<long long>(*walker - '0');
        walker++;
    }
    value = result_value;
    return (true);
}

bool api_http_parse_hex(const char *start, const char *end,
    long long &value)
{
    const char *walker;
    long long result_value;

    if (!start || !end)
        return (false);
    walker = start;
    if (walker == end)
        return (false);
    result_value = 0;
    while (walker < end)
    {
        char lower_character;

        if (*walker == ';')
            break;
        lower_character = api_http_to_lower(*walker);
        if (lower_character >= '0' && lower_character <= '9')
            lower_character = static_cast<char>(lower_character - '0');
        else if (lower_character >= 'a' && lower_character <= 'f')
            lower_character = static_cast<char>(lower_character - 'a' + 10);
        else
            return (false);
        if (result_value > (LLONG_MAX >> 4))
            return (false);
        result_value <<= 4;
        if (result_value > (LLONG_MAX - lower_character))
            return (false);
        result_value += lower_character;
        walker++;
    }
    value = result_value;
    return (true);
}

void api_http_parse_headers(const char *headers_start,
    const char *headers_end, bool &connection_close,
    bool &chunked_encoding, bool &has_length, long long &content_length)
{
    const char *walker;
    bool first_line;

    connection_close = false;
    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    walker = headers_start;
    first_line = true;
    while (walker < headers_end)
    {
        const char *line_end;
        size_t line_length;

        line_end = api_http_find_crlf(walker, headers_end);
        if (!line_end)
            return ;
        line_length = static_cast<size_t>(line_end - walker);
        if (first_line)
        {
            if (line_length >= 8)
            {
                const char *version_start;
                bool http11;

                version_start = walker;
                http11 = false;
                if (api_http_to_lower(version_start[0]) == 'h' &&
                    api_http_to_lower(version_start[1]) == 't' &&
                    api_http_to_lower(version_start[2]) == 't' &&
                    api_http_to_lower(version_start[3]) == 'p' &&
                    version_start[4] == '/' &&
                    version_start[5] == '1' &&
                    version_start[6] == '.' &&
                    version_start[7] == '1')
                    http11 = true;
                if (!http11)
                    connection_close = true;
            }
            first_line = false;
        }
        else if (line_length > 0)
        {
            if (api_http_line_starts_with(walker, line_length, "content-length"))
            {
                const char *value_start;
                const char *value_end;
                long long parsed_length;

                value_start = walker;
                value_start += ft_strlen("Content-Length:");
                value_end = walker + line_length;
                api_http_trim_header_value(&value_start, &value_end);
                if (api_http_parse_decimal(value_start, value_end, parsed_length))
                {
                    has_length = true;
                    content_length = parsed_length;
                }
            }
            else if (api_http_line_starts_with(walker, line_length, "connection"))
            {
                const char *value_start;
                const char *value_end;
                const char *iterator;
                bool saw_close;

                value_start = walker;
                value_start += ft_strlen("Connection:");
                value_end = walker + line_length;
                api_http_trim_header_value(&value_start, &value_end);
                iterator = value_start;
                saw_close = false;
                while (iterator < value_end)
                {
                    char lower_character;

                    lower_character = api_http_to_lower(*iterator);
                    if (lower_character == 'c')
                    {
                        const char *check_pointer;

                        check_pointer = iterator;
                        if ((value_end - check_pointer) >= 5 &&
                            api_http_to_lower(check_pointer[0]) == 'c' &&
                            api_http_to_lower(check_pointer[1]) == 'l' &&
                            api_http_to_lower(check_pointer[2]) == 'o' &&
                            api_http_to_lower(check_pointer[3]) == 's' &&
                            api_http_to_lower(check_pointer[4]) == 'e')
                        {
                            saw_close = true;
                            break;
                        }
                    }
                    iterator++;
                }
                if (saw_close)
                    connection_close = true;
                else if (!has_length && !chunked_encoding)
                {
                    const char *keep_pointer;

                    keep_pointer = value_start;
                    while (keep_pointer < value_end)
                    {
                        if (api_http_to_lower(*keep_pointer) == 'k')
                        {
                            if ((value_end - keep_pointer) >= 10 &&
                                api_http_to_lower(keep_pointer[0]) == 'k' &&
                                api_http_to_lower(keep_pointer[1]) == 'e' &&
                                api_http_to_lower(keep_pointer[2]) == 'e' &&
                                api_http_to_lower(keep_pointer[3]) == 'p' &&
                                keep_pointer[4] == '-' &&
                                api_http_to_lower(keep_pointer[5]) == 'a' &&
                                api_http_to_lower(keep_pointer[6]) == 'l' &&
                                api_http_to_lower(keep_pointer[7]) == 'i' &&
                                api_http_to_lower(keep_pointer[8]) == 'v' &&
                                api_http_to_lower(keep_pointer[9]) == 'e')
                            {
                                connection_close = false;
                                break;
                            }
                        }
                        keep_pointer++;
                    }
                }
            }
            else if (api_http_line_starts_with(walker, line_length,
                    "transfer-encoding"))
            {
                const char *value_start;
                const char *value_end;
                const char *iterator;

                value_start = walker;
                value_start += ft_strlen("Transfer-Encoding:");
                value_end = walker + line_length;
                api_http_trim_header_value(&value_start, &value_end);
                iterator = value_start;
                while (iterator < value_end)
                {
                    if (api_http_to_lower(*iterator) == 'c')
                    {
                        if ((value_end - iterator) >= 7 &&
                            api_http_to_lower(iterator[0]) == 'c' &&
                            api_http_to_lower(iterator[1]) == 'h' &&
                            api_http_to_lower(iterator[2]) == 'u' &&
                            api_http_to_lower(iterator[3]) == 'n' &&
                            api_http_to_lower(iterator[4]) == 'k' &&
                            api_http_to_lower(iterator[5]) == 'e' &&
                            api_http_to_lower(iterator[6]) == 'd')
                        {
                            chunked_encoding = true;
                            break;
                        }
                    }
                    iterator++;
                }
            }
        }
        if (line_end == headers_end)
            break;
        walker = line_end + 2;
    }
    return ;
}

bool api_http_chunked_body_complete(const char *body_start,
    size_t body_size, size_t &consumed_length)
{
    size_t offset;

    consumed_length = 0;
    if (!body_start)
        return (false);
    offset = 0;
    while (offset < body_size)
    {
        const char *line_end;
        long long chunk_size_value;
        size_t line_length;

        line_end = api_http_find_crlf(body_start + offset,
                body_start + body_size);
        if (!line_end)
            return (false);
        line_length = static_cast<size_t>(line_end - (body_start + offset));
        if (!api_http_parse_hex(body_start + offset,
                body_start + offset + line_length, chunk_size_value))
            return (false);
        offset += line_length;
        offset += 2;
        if (chunk_size_value < 0)
            return (false);
        if (chunk_size_value == 0)
        {
            while (true)
            {
                const char *trailer_end;
                size_t trailer_length;

                if (offset > body_size)
                    return (false);
                trailer_end = api_http_find_crlf(body_start + offset,
                        body_start + body_size);
                if (!trailer_end)
                    return (false);
                trailer_length = static_cast<size_t>(trailer_end - (body_start + offset));
                offset += trailer_length;
                offset += 2;
                if (trailer_length == 0)
                {
                    consumed_length = offset;
                    return (true);
                }
            }
        }
        if (static_cast<size_t>(chunk_size_value) > (body_size - offset))
            return (false);
        offset += static_cast<size_t>(chunk_size_value);
        if ((body_size - offset) < 2)
            return (false);
        if (body_start[offset] != '\r' || body_start[offset + 1] != '\n')
            return (false);
        offset += 2;
    }
    return (false);
}

bool api_http_decode_chunked(const char *body_start, size_t body_size,
    ft_string &decoded_body, size_t &consumed_length)
{
    size_t offset;

    decoded_body.clear();
    consumed_length = 0;
    if (!body_start)
        return (false);
    offset = 0;
    while (offset < body_size)
    {
        const char *line_end;
        long long chunk_size_value;
        size_t line_length;
        size_t chunk_size_t;

        line_end = api_http_find_crlf(body_start + offset,
                body_start + body_size);
        if (!line_end)
            return (false);
        line_length = static_cast<size_t>(line_end - (body_start + offset));
        if (!api_http_parse_hex(body_start + offset,
                body_start + offset + line_length, chunk_size_value))
            return (false);
        offset += line_length;
        offset += 2;
        if (chunk_size_value < 0)
            return (false);
        chunk_size_t = static_cast<size_t>(chunk_size_value);
        if (chunk_size_t == 0)
        {
            while (true)
            {
                const char *trailer_end;
                size_t trailer_length;

                if (offset > body_size)
                    return (false);
                trailer_end = api_http_find_crlf(body_start + offset,
                        body_start + body_size);
                if (!trailer_end)
                    return (false);
                trailer_length = static_cast<size_t>(trailer_end - (body_start + offset));
                offset += trailer_length;
                offset += 2;
                if (trailer_length == 0)
                {
                    consumed_length = offset;
                    return (true);
                }
            }
        }
        if (chunk_size_t > (body_size - offset))
            return (false);
        size_t index;

        index = 0;
        while (index < chunk_size_t)
        {
            decoded_body.append(body_start[offset + index]);
            if (decoded_body.get_error())
                return (false);
            index++;
        }
        offset += chunk_size_t;
        if ((body_size - offset) < 2)
            return (false);
        if (body_start[offset] != '\r' || body_start[offset + 1] != '\n')
            return (false);
        offset += 2;
    }
    return (false);
}

static bool api_http_measure_add(size_t &total, size_t increment)
{
    if (SIZE_MAX - total < increment)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    total += increment;
    return (true);
}

bool api_http_measure_json_payload(json_group *payload,
    size_t &payload_length)
{
    size_t total_length;
    json_group *group_iterator;
    int entry_errno;

    entry_errno = ft_errno;
    payload_length = 0;
    if (!payload)
    {
        ft_errno = entry_errno;
        return (true);
    }
    total_length = 0;
    if (!api_http_measure_add(total_length, 2))
        return (false);
    group_iterator = payload;
    while (group_iterator)
    {
        const char *group_name;
        size_t name_length;
        json_item *item_iterator;

        group_name = group_iterator->name;
        if (!group_name)
            group_name = "";
        name_length = ft_strlen(group_name);
        if (!api_http_measure_add(total_length, 3))
            return (false);
        if (!api_http_measure_add(total_length, name_length))
            return (false);
        if (!api_http_measure_add(total_length, 5))
            return (false);
        item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *item_key;
            size_t key_length;
            bool big_number_value;
            const char *value_text;
            size_t value_length;

            item_key = item_iterator->key;
            if (!item_key)
                item_key = "";
            key_length = ft_strlen(item_key);
            if (!api_http_measure_add(total_length, 5))
                return (false);
            if (!api_http_measure_add(total_length, key_length))
                return (false);
            if (!api_http_measure_add(total_length, 3))
                return (false);
            big_number_value = (item_iterator->is_big_number == true)
                && (item_iterator->big_number != ft_nullptr);
            if (big_number_value)
                value_text = item_iterator->big_number->c_str();
            else
                value_text = item_iterator->value;
            if (!value_text)
                value_text = "";
            value_length = ft_strlen(value_text);
            if (!big_number_value)
            {
                if (!api_http_measure_add(total_length, 1))
                    return (false);
            }
            if (!api_http_measure_add(total_length, value_length))
                return (false);
            if (!big_number_value)
            {
                if (!api_http_measure_add(total_length, 1))
                    return (false);
            }
            if (item_iterator->next)
            {
                if (!api_http_measure_add(total_length, 2))
                    return (false);
            }
            else
            {
                if (!api_http_measure_add(total_length, 1))
                    return (false);
            }
            item_iterator = item_iterator->next;
        }
        if (group_iterator->next)
        {
            if (!api_http_measure_add(total_length, 5))
                return (false);
        }
        else
        {
            if (!api_http_measure_add(total_length, 4))
                return (false);
        }
        group_iterator = group_iterator->next;
    }
    if (!api_http_measure_add(total_length, 2))
        return (false);
    payload_length = total_length;
    ft_errno = entry_errno;
    return (true);
}

static bool api_http_stream_literal(api_http_send_callback send_callback,
    void *context, const char *literal, size_t literal_length, int &error_code)
{
    if (literal_length == 0)
        return (true);
    if (!send_callback)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (!send_callback(literal, literal_length, context, error_code))
        return (false);
    return (true);
}

bool api_http_stream_json_payload(json_group *payload,
    api_http_send_callback send_callback, void *context, int &error_code)
{
    json_group *group_iterator;
    int entry_errno;

    entry_errno = ft_errno;
    error_code = FT_ERR_SUCCESSS;
    if (!payload)
    {
        ft_errno = entry_errno;
        return (true);
    }
    if (!api_http_stream_literal(send_callback, context, "{\n",
            ft_strlen("{\n"), error_code))
        return (false);
    group_iterator = payload;
    while (group_iterator)
    {
        const char *group_name;
        size_t group_name_length;
        json_item *item_iterator;

        group_name = group_iterator->name;
        if (!group_name)
            group_name = "";
        group_name_length = ft_strlen(group_name);
        if (!api_http_stream_literal(send_callback, context, "  \"",
                ft_strlen("  \""), error_code))
            return (false);
        if (group_name_length > 0)
        {
            if (!api_http_stream_literal(send_callback, context, group_name,
                    group_name_length, error_code))
                return (false);
        }
        if (!api_http_stream_literal(send_callback, context, "\": {\n",
                ft_strlen("\": {\n"), error_code))
            return (false);
        item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *item_key;
            size_t item_key_length;
            bool big_number_value;
            const char *value_text;
            size_t value_length;

            item_key = item_iterator->key;
            if (!item_key)
                item_key = "";
            item_key_length = ft_strlen(item_key);
            if (!api_http_stream_literal(send_callback, context, "    \"",
                    ft_strlen("    \""), error_code))
                return (false);
            if (item_key_length > 0)
            {
                if (!api_http_stream_literal(send_callback, context, item_key,
                        item_key_length, error_code))
                    return (false);
            }
            if (!api_http_stream_literal(send_callback, context, "\": ",
                    ft_strlen("\": "), error_code))
                return (false);
            big_number_value = (item_iterator->is_big_number == true)
                && (item_iterator->big_number != ft_nullptr);
            if (big_number_value)
                value_text = item_iterator->big_number->c_str();
            else
                value_text = item_iterator->value;
            if (!value_text)
                value_text = "";
            value_length = ft_strlen(value_text);
            if (!big_number_value)
            {
                if (!api_http_stream_literal(send_callback, context, "\"",
                        ft_strlen("\""), error_code))
                    return (false);
            }
            if (value_length > 0)
            {
                if (!api_http_stream_literal(send_callback, context, value_text,
                        value_length, error_code))
                    return (false);
            }
            if (!big_number_value)
            {
                if (!api_http_stream_literal(send_callback, context, "\"",
                        ft_strlen("\""), error_code))
                    return (false);
            }
            if (item_iterator->next)
            {
                if (!api_http_stream_literal(send_callback, context, ",\n",
                        ft_strlen(",\n"), error_code))
                    return (false);
            }
            else
            {
                if (!api_http_stream_literal(send_callback, context, "\n",
                        ft_strlen("\n"), error_code))
                    return (false);
            }
            item_iterator = item_iterator->next;
        }
        if (group_iterator->next)
        {
            if (!api_http_stream_literal(send_callback, context, "  },\n",
                    ft_strlen("  },\n"), error_code))
                return (false);
        }
        else
        {
            if (!api_http_stream_literal(send_callback, context, "  }\n",
                    ft_strlen("  }\n"), error_code))
                return (false);
        }
        group_iterator = group_iterator->next;
    }
    if (!api_http_stream_literal(send_callback, context, "}\n",
            ft_strlen("}\n"), error_code))
        return (false);
    ft_errno = entry_errno;
    return (true);
}

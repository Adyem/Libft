#include "logger_internal.hpp"
#include <unistd.h>
#include <cerrno>
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"

struct s_json_sink_field
{
    ft_string key;
    ft_string value;
    ft_bool has_value;
    ft_bool value_is_json;

    s_json_sink_field()
        : key(), value(), has_value(FT_FALSE), value_is_json(FT_FALSE)
    {
        return ;
    }

    int32_t initialize(const s_json_sink_field &other) noexcept
    {
        int32_t error_code;

        error_code = this->key.initialize(other.key);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = this->value.initialize(other.value);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        this->has_value = other.has_value;
        this->value_is_json = other.value_is_json;
        return (FT_ERR_SUCCESS);
    }

    int32_t destroy() noexcept
    {
        int32_t first_error;
        int32_t error_code;

        first_error = this->key.destroy();
        error_code = this->value.destroy();
        if (first_error == FT_ERR_SUCCESS && error_code != FT_ERR_SUCCESS)
            first_error = error_code;
        this->has_value = FT_FALSE;
        this->value_is_json = FT_FALSE;
        return (first_error);
    }
};

static int32_t logger_json_sink_append_literal(ft_string &buffer, const char *literal)
{
    if (!literal)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (buffer.append(literal));
}

static int32_t logger_json_sink_append_character_sequence(ft_string &buffer, const char *sequence)
{
    if (!sequence)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (buffer.append(sequence));
}

static int32_t logger_json_sink_append_json_escaped(ft_string &buffer, char character)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    int32_t error_code_value;

    if (character == '\\' || character == '"')
    {
        error_code_value = buffer.append('\\');
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        return (buffer.append(character));
    }
    if (character >= 0 && character < 0x20)
    {
        char escape_buffer[7];

        escape_buffer[0] = '\\';
        escape_buffer[1] = 'u';
        escape_buffer[2] = '0';
        escape_buffer[3] = '0';
        escape_buffer[4] = hex_digits[(static_cast<unsigned char>(character) >> 4) & 0x0F];
        escape_buffer[5] = hex_digits[static_cast<unsigned char>(character) & 0x0F];
        escape_buffer[6] = '\0';
        return (logger_json_sink_append_literal(buffer, escape_buffer));
    }
    return (buffer.append(character));
}

static int32_t logger_json_sink_append_json_string(ft_string &buffer, const char *value)
{
    ft_size_t entry_index;
    int32_t error_code_value;

    if (!value)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    error_code_value = buffer.append('"');
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    entry_index = 0;
    while (value[entry_index] != '\0')
    {
        error_code_value = logger_json_sink_append_json_escaped(buffer, value[entry_index]);
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        entry_index += 1;
    }
    return (buffer.append('"'));
}

static int32_t logger_json_sink_hex_value(char character)
{
    if (character >= '0' && character <= '9')
        return (character - '0');
    if (character >= 'a' && character <= 'f')
        return (10 + character - 'a');
    if (character >= 'A' && character <= 'F')
        return (10 + character - 'A');
    return (FT_ERR_INVALID_ARGUMENT);
}

static void logger_json_sink_decode_quoted(const char *message, ft_size_t &entry_index, char *destination, ft_size_t capacity)
{
    ft_size_t dest_index;

    dest_index = 0;
    while (message[entry_index] != '\0' && message[entry_index] != '"')
    {
        char character;

        character = message[entry_index];
        if (character == '\\' && message[entry_index + 1] != '\0')
        {
            entry_index += 1;
            character = message[entry_index];
            if (character == 'x' && message[entry_index + 1] != '\0' && message[entry_index + 2] != '\0')
            {
                int32_t high_value;
                int32_t low_value;

                high_value = logger_json_sink_hex_value(message[entry_index + 1]);
                low_value = logger_json_sink_hex_value(message[entry_index + 2]);
                if (high_value >= 0 && low_value >= 0)
                {
                    character = static_cast<char>((high_value << 4) | low_value);
                    entry_index += 2;
                }
            }
        }
        if (dest_index + 1 < capacity)
        {
            destination[dest_index] = character;
            dest_index += 1;
        }
        entry_index += 1;
    }
    destination[dest_index] = '\0';
    if (message[entry_index] == '"')
        entry_index += 1;
    return ;
}

static void logger_json_sink_decode_unquoted(const char *message, ft_size_t &entry_index, char *destination, ft_size_t capacity)
{
    ft_size_t dest_index;

    dest_index = 0;
    while (message[entry_index] != '\0' && message[entry_index] != ' ' && message[entry_index] != '\n')
    {
        if (dest_index + 1 < capacity)
        {
            destination[dest_index] = message[entry_index];
            dest_index += 1;
        }
        entry_index += 1;
    }
    destination[dest_index] = '\0';
    return ;
}

int32_t ft_log_add_sink(t_log_sink sink, void *user_data)
{
    if (!sink)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    s_log_sink entry;
    int32_t prepare_status;
    int32_t lock_error;

    prepare_status = log_sink_prepare_thread_safety(&entry);
    if (prepare_status != FT_ERR_SUCCESS)
    {
        return (prepare_status);
    }
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        log_sink_teardown_thread_safety(&entry);
        return (lock_error);
    }
    entry.function = sink;
    entry.user_data = user_data;
    g_sinks.push_back(entry);
    if (g_sinks.get_error() != FT_ERR_SUCCESS)
    {
        log_sink_teardown_thread_safety(&entry);
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return (lock_error);
        }
        return (g_sinks.get_error());
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_json_sink(const char *message, void *user_data)
{
    char time_buffer[64];
    char level_buffer[16];
    char severity_buffer[16];
    char key_buffer[128];
    char value_buffer[1024];
    char message_buffer[2048];
    ft_vector<s_json_sink_field> context_fields;
    ft_size_t entry_index;
    int32_t file_descriptor;
    ft_bool message_is_json;
    int32_t error_code_value;

    if (!message)
        return (FT_ERR_INVALID_ARGUMENT);
    time_buffer[0] = '\0';
    level_buffer[0] = '\0';
    severity_buffer[0] = '\0';
    message_buffer[0] = '\0';
    message_is_json = FT_FALSE;
    if (context_fields.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    file_descriptor = 1;
    if (user_data)
        file_descriptor = *static_cast<int32_t *>(user_data);
    entry_index = 0;
    while (message[entry_index] == ' ')
        entry_index += 1;
    while (message[entry_index] != '\0' && message[entry_index] != '\n')
    {
        ft_size_t key_length;
        ft_bool has_value;
        ft_bool value_is_json;

        key_length = 0;
        while (message[entry_index] != '\0' && message[entry_index] != '=' && message[entry_index] != ' ' && message[entry_index] != '\n')
        {
            if (key_length + 1 < sizeof(key_buffer))
            {
                key_buffer[key_length] = message[entry_index];
                key_length += 1;
            }
            entry_index += 1;
        }
        key_buffer[key_length] = '\0';
        while (message[entry_index] == ' ')
            entry_index += 1;
        has_value = FT_FALSE;
        value_is_json = FT_FALSE;
        value_buffer[0] = '\0';
        if (message[entry_index] == '=')
        {
            has_value = FT_TRUE;
            entry_index += 1;
            if (message[entry_index] == '"')
            {
                entry_index += 1;
                logger_json_sink_decode_quoted(message, entry_index, value_buffer, sizeof(value_buffer));
            }
            else
                logger_json_sink_decode_unquoted(message, entry_index, value_buffer, sizeof(value_buffer));
            if (value_buffer[0] == '{' || value_buffer[0] == '[')
                value_is_json = FT_TRUE;
        }
        while (message[entry_index] == ' ')
            entry_index += 1;
        if (key_buffer[0] == '\0')
        {
            if (message[entry_index] == '\0' || message[entry_index] == '\n')
                break ;
            continue ;
        }
        if (ft_strcmp(key_buffer, "time") == 0 && has_value)
            ft_strlcpy(time_buffer, value_buffer, sizeof(time_buffer));
        else if (ft_strcmp(key_buffer, "level") == 0 && has_value)
            ft_strlcpy(level_buffer, value_buffer, sizeof(level_buffer));
        else if (ft_strcmp(key_buffer, "severity") == 0 && has_value)
            ft_strlcpy(severity_buffer, value_buffer, sizeof(severity_buffer));
        else if (ft_strcmp(key_buffer, "message") == 0 && has_value)
        {
            ft_strlcpy(message_buffer, value_buffer, sizeof(message_buffer));
            message_is_json = value_is_json;
        }
        else
        {
            s_json_sink_field field;

            if (field.key.initialize(key_buffer) != FT_ERR_SUCCESS)
                return (FT_ERR_NO_MEMORY);
            field.has_value = has_value;
            field.value_is_json = value_is_json;
            if (field.value.initialize(value_buffer) != FT_ERR_SUCCESS)
                return (FT_ERR_NO_MEMORY);
            context_fields.push_back(field);
            if (context_fields.get_error() != FT_ERR_SUCCESS)
                return (context_fields.get_error());
        }
        if (message[entry_index] == '\0' || message[entry_index] == '\n')
            break ;
    }
    ft_string payload;
    if (payload.initialize("{") != FT_ERR_SUCCESS)
    {
        return (FT_ERR_NO_MEMORY);
    }
    error_code_value = logger_json_sink_append_literal(payload, "\"time\":");
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    error_code_value = logger_json_sink_append_json_string(payload, time_buffer);
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    error_code_value = logger_json_sink_append_literal(payload, ",\"level\":");
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    error_code_value = logger_json_sink_append_json_string(payload, level_buffer);
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    if (severity_buffer[0] != '\0')
    {
        error_code_value = logger_json_sink_append_literal(payload, ",\"severity\":");
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        error_code_value = logger_json_sink_append_character_sequence(payload, severity_buffer);
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
    }
    error_code_value = logger_json_sink_append_literal(payload, ",\"message\":");
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    if (message_is_json)
    {
        error_code_value = logger_json_sink_append_character_sequence(payload, message_buffer);
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
    }
    else
    {
        error_code_value = logger_json_sink_append_json_string(payload, message_buffer);
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
    }
    ft_size_t context_count;
    ft_size_t context_index;

    context_count = context_fields.size();
    if (context_fields.get_error() != FT_ERR_SUCCESS)
    {
        return (context_fields.get_error());
    }
    context_index = 0;
    while (context_index < context_count)
    {
        const s_json_sink_field &field = context_fields[context_index];

        error_code_value = logger_json_sink_append_literal(payload, ",\"");
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        error_code_value = logger_json_sink_append_character_sequence(payload, field.key.c_str());
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        error_code_value = logger_json_sink_append_literal(payload, "\":");
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        if (field.has_value)
        {
            if (field.value_is_json)
            {
                error_code_value = logger_json_sink_append_character_sequence(payload, field.value.c_str());
                if (error_code_value != FT_ERR_SUCCESS)
                    return (error_code_value);
            }
            else
            {
                error_code_value = logger_json_sink_append_json_string(payload, field.value.c_str());
                if (error_code_value != FT_ERR_SUCCESS)
                    return (error_code_value);
            }
        }
        else
        {
            error_code_value = logger_json_sink_append_literal(payload, "true");
            if (error_code_value != FT_ERR_SUCCESS)
                return (error_code_value);
        }
        context_index += 1;
    }
    error_code_value = logger_json_sink_append_literal(payload, "}\n");
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    ssize_t write_result;

    write_result = write(file_descriptor, payload.c_str(), payload.size());
    if (write_result < 0)
        return (FT_ERR_IO);
    if (write_result != static_cast<ssize_t>(payload.size()))
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

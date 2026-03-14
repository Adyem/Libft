#include "logger_internal.hpp"

static void logger_append_literal(ft_string &buffer, const char *literal,
    int32_t &error_code_value)
{
    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    if (!literal)
    {
        error_code_value = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    buffer.append(literal);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        error_code_value = ft_string::get_error();
    return ;
}

static void logger_append_character_sequence(ft_string &buffer, const char *sequence,
    int32_t &error_code_value)
{
    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    buffer.append(sequence);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        error_code_value = ft_string::get_error();
    return ;
}

static void logger_append_json_escaped(ft_string &buffer, char character,
    int32_t &error_code_value)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    if (character == '\\' || character == '"')
    {
        buffer.append('\\');
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            error_code_value = ft_string::get_error();
            return ;
        }
        buffer.append(character);
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            error_code_value = ft_string::get_error();
        return ;
    }
    if (character == '\b')
    {
        logger_append_literal(buffer, "\\b", error_code_value);
        return ;
    }
    if (character == '\f')
    {
        logger_append_literal(buffer, "\\f", error_code_value);
        return ;
    }
    if (character == '\n')
    {
        logger_append_literal(buffer, "\\n", error_code_value);
        return ;
    }
    if (character == '\r')
    {
        logger_append_literal(buffer, "\\r", error_code_value);
        return ;
    }
    if (character == '\t')
    {
        logger_append_literal(buffer, "\\t", error_code_value);
        return ;
    }
    unsigned char code;

    code = static_cast<unsigned char>(character);
    if (code < 0x20)
    {
        char escape_buffer[7];

        escape_buffer[0] = '\\';
        escape_buffer[1] = 'u';
        escape_buffer[2] = '0';
        escape_buffer[3] = '0';
        escape_buffer[4] = hex_digits[(code >> 4) & 0x0F];
        escape_buffer[5] = hex_digits[code & 0x0F];
        escape_buffer[6] = '\0';
        logger_append_character_sequence(buffer, escape_buffer, error_code_value);
        return ;
    }
    buffer.append(character);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        error_code_value = ft_string::get_error();
    return ;
}

static void logger_append_json_string(ft_string &buffer, const char *value,
    int32_t &error_code_value)
{
    ft_size_t entry_index;

    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    if (!value)
    {
        error_code_value = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    buffer.append('"');
    if (ft_string::get_error() != FT_ERR_SUCCESS)
    {
        error_code_value = ft_string::get_error();
        return ;
    }
    entry_index = 0;
    while (value[entry_index] != '\0' && error_code_value == FT_ERR_SUCCESS)
    {
        logger_append_json_escaped(buffer, value[entry_index], error_code_value);
        entry_index += 1;
    }
    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    buffer.append('"');
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        error_code_value = ft_string::get_error();
    return ;
}

static void logger_append_json_optional_string(ft_string &buffer,
    const char *value, int32_t &error_code_value)
{
    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    if (!value)
    {
        logger_append_literal(buffer, "null", error_code_value);
        return ;
    }
    logger_append_json_string(buffer, value, error_code_value);
    return ;
}

static void logger_append_field(ft_string &buffer, const s_log_field &field,
    ft_bool &first_field, int32_t &error_code_value)
{
    ft_bool lock_acquired;

    if (error_code_value != FT_ERR_SUCCESS)
        return ;
    lock_acquired = FT_FALSE;
    if (log_field_lock(&field, &lock_acquired) != 0)
    {
        error_code_value = FT_ERR_SUCCESS;
        return ;
    }
    if (!field.key)
    {
        error_code_value = FT_ERR_INVALID_ARGUMENT;
        log_field_unlock(&field, lock_acquired);
        return ;
    }
    if (!first_field)
        logger_append_literal(buffer, ",", error_code_value);
    else
        first_field = FT_FALSE;
    if (error_code_value != FT_ERR_SUCCESS)
    {
        log_field_unlock(&field, lock_acquired);
        return ;
    }
    logger_append_json_string(buffer, field.key, error_code_value);
    logger_append_literal(buffer, ":", error_code_value);
    logger_append_json_optional_string(buffer, field.value, error_code_value);
    log_field_unlock(&field, lock_acquired);
    return ;
}

static void logger_dispatch_structured(t_log_level level, const char *payload)
{
    if (level == LOG_LEVEL_DEBUG)
        ft_log_debug("%s", payload);
    else if (level == LOG_LEVEL_INFO)
        ft_log_info("%s", payload);
    else if (level == LOG_LEVEL_WARN)
        ft_log_warn("%s", payload);
    else
        ft_log_error("%s", payload);
    return ;
}

void ft_log_structured(t_log_level level, const char *message,
    const s_log_field *fields, ft_size_t field_count)
{
    ft_string payload;
    int32_t error_code_value;
    ft_size_t entry_index;
    ft_bool first_field;
    ft_vector<s_log_context_view> context_snapshot;

    if (!message)
    {
        return ;
    }
    if (field_count != 0 && !fields)
    {
        return ;
    }
    error_code_value = FT_ERR_SUCCESS;
    logger_append_literal(payload, "{\"message\":", error_code_value);
    logger_append_json_string(payload, message, error_code_value);
    logger_append_literal(payload, ",\"fields\":{", error_code_value);
    first_field = FT_TRUE;
    entry_index = 0;
    while (entry_index < field_count && error_code_value == FT_ERR_SUCCESS)
    {
        logger_append_field(payload, fields[entry_index], first_field, error_code_value);
        entry_index += 1;
    }
    if (error_code_value == FT_ERR_SUCCESS)
    {
        if (logger_context_snapshot(context_snapshot) != 0)
            error_code_value = FT_ERR_SUCCESS;
    }
    if (error_code_value == FT_ERR_SUCCESS)
    {
        ft_size_t context_count;
        ft_size_t context_index;

        context_count = context_snapshot.size();
        if (context_snapshot.get_error() != FT_ERR_SUCCESS)
            error_code_value = context_snapshot.get_error();
        context_index = 0;
        while (context_index < context_count && error_code_value == FT_ERR_SUCCESS)
        {
            const s_log_context_view &view = context_snapshot[context_index];
            s_log_field context_field;

            if (context_snapshot.get_error() != FT_ERR_SUCCESS)
            {
                error_code_value = context_snapshot.get_error();
                break ;
            }
            context_field.key = view.key;
            if (view.has_value)
                context_field.value = view.value;
            else
                context_field.value = ft_nullptr;
            logger_append_field(payload, context_field, first_field, error_code_value);
            context_index += 1;
        }
    }
    logger_append_literal(payload, "}}", error_code_value);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (ft_string::get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    logger_dispatch_structured(level, payload.c_str());
    return ;
}

void ft_log_debug_structured(const char *message, const s_log_field *fields,
    ft_size_t field_count)
{
    ft_log_structured(LOG_LEVEL_DEBUG, message, fields, field_count);
    return ;
}

void ft_log_info_structured(const char *message, const s_log_field *fields,
    ft_size_t field_count)
{
    ft_log_structured(LOG_LEVEL_INFO, message, fields, field_count);
    return ;
}

void ft_log_warn_structured(const char *message, const s_log_field *fields,
    ft_size_t field_count)
{
    ft_log_structured(LOG_LEVEL_WARN, message, fields, field_count);
    return ;
}

void ft_log_error_structured(const char *message, const s_log_field *fields,
    ft_size_t field_count)
{
    ft_log_structured(LOG_LEVEL_ERROR, message, fields, field_count);
    return ;
}

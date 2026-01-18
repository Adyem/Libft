#include "logger_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static thread_local ft_vector<s_log_context_entry> g_log_context_entries;

static int logger_context_rollback(size_t count)
{
    size_t index;

    index = 0;
    while (index < count)
    {
        g_log_context_entries.pop_back();
        int vector_error = g_log_context_entries.get_error();
        if (vector_error != FT_ERR_SUCCESSS)
            return (vector_error);
        index += 1;
    }
    return (FT_ERR_SUCCESSS);
}

int logger_context_push(const s_log_field *fields, size_t field_count,
        size_t *pushed_count)
{
    size_t index;

    if (!pushed_count)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    *pushed_count = 0;
    if (field_count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (!fields)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    index = 0;
    while (index < field_count)
    {
        const s_log_field *field;
        s_log_context_entry entry;
        bool lock_acquired;
        int error_code;

        field = &fields[index];
        lock_acquired = false;
        error_code = log_field_lock(field, &lock_acquired);
        if (error_code != FT_ERR_SUCCESSS)
        {
            logger_context_rollback(index);
            return (-1);
        }
        if (!field->key)
        {
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(index);
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        entry.key = field->key;
        error_code = entry.key.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(index);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (field->value)
        {
            entry.value = field->value;
            error_code = entry.value.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                log_field_unlock(field, lock_acquired);
                logger_context_rollback(index);
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            entry.has_value = true;
        }
        else
        {
            entry.value.clear();
            entry.has_value = false;
        }
        log_field_unlock(field, lock_acquired);
        g_log_context_entries.push_back(entry);
        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            logger_context_rollback(index);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        index += 1;
    }
    *pushed_count = field_count;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void logger_context_pop(size_t count)
{
    size_t available;
    int error_code;

    available = g_log_context_entries.size();
    error_code = g_log_context_entries.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (count > available)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        count = available;
    }
    else
        error_code = FT_ERR_SUCCESSS;
    while (count > 0)
    {
        g_log_context_entries.pop_back();
        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return ;
        }
        count -= 1;
    }
    ft_global_error_stack_push(error_code);
    return ;
}

static int logger_context_format_prefix(ft_string &prefix)
{
    size_t count;
    size_t index;
    bool first_entry;
    int error_code;

    count = g_log_context_entries.size();
    error_code = g_log_context_entries.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    prefix.append('[');
    error_code = prefix.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    index = 0;
    first_entry = true;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];

        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (!first_entry)
        {
            prefix.append(' ');
            error_code = prefix.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else
            first_entry = false;
        prefix.append(entry.key);
        error_code = prefix.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (entry.has_value)
        {
            prefix.append('=');
            error_code = prefix.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            prefix.append(entry.value);
            error_code = prefix.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        index += 1;
    }
    prefix.append(']');
    error_code = prefix.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    prefix.append(' ');
    error_code = prefix.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (1);
}

int logger_context_apply_plain(ft_string &text)
{
    ft_string prefix;
    int result;

    result = logger_context_format_prefix(prefix);
    if (result <= 0)
    {
        if (result == 0)
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (result);
    }
    if (prefix.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(prefix.get_error());
        return (-1);
    }
    ft_string combined(prefix);
    int error_code = combined.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    combined.append(text);
    error_code = combined.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    text = combined;
    error_code = text.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

static bool logger_context_value_needs_quotes(const char *value)
{
    size_t index;

    if (!value)
        return (false);
    index = 0;
    while (value[index] != '\0')
    {
        unsigned char character;

        character = static_cast<unsigned char>(value[index]);
        if (character <= ' ' || character == '"' || character == '\\' || character == '=')
            return (true);
        index += 1;
    }
    return (false);
}

static int logger_context_append_flat_value(ft_string &output, const char *value)
{
    size_t index;
    static const char hex_digits[] = "0123456789ABCDEF";
    int error_code;

    if (!value)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!logger_context_value_needs_quotes(value))
    {
        output.append(value);
        error_code = output.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    output.append('"');
    error_code = output.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    index = 0;
    while (value[index] != '\0')
    {
        char character;

        character = value[index];
        if (character == '"' || character == '\\')
        {
            output.append('\\');
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            output.append(character);
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else if (static_cast<unsigned char>(character) < 0x20)
        {
            char escape_buffer[5];

            escape_buffer[0] = '\\';
            escape_buffer[1] = 'x';
            escape_buffer[2] = hex_digits[(static_cast<unsigned char>(character) >> 4) & 0x0F];
            escape_buffer[3] = hex_digits[static_cast<unsigned char>(character) & 0x0F];
            escape_buffer[4] = '\0';
            output.append(escape_buffer);
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else
        {
            output.append(character);
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        index += 1;
    }
    output.append('"');
    error_code = output.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int logger_context_format_flat(ft_string &output)
{
    size_t count;
    size_t index;
    bool first_entry;
    int error_code;

    output.clear();
    error_code = output.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    count = g_log_context_entries.size();
    error_code = g_log_context_entries.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    index = 0;
    first_entry = true;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];

        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (!first_entry)
        {
            output.append(' ');
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else
            first_entry = false;
        output.append(entry.key);
        error_code = output.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (entry.has_value)
        {
            output.append('=');
            error_code = output.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            error_code = entry.value.get_error();
            if (error_code != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            if (logger_context_append_flat_value(output, entry.value.c_str()) != 0)
                return (-1);
        }
        index += 1;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int logger_context_snapshot(ft_vector<s_log_context_view> &snapshot)
{
    size_t count;
    size_t index;
    int error_code;

    snapshot.clear();
    error_code = snapshot.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    count = g_log_context_entries.size();
    error_code = g_log_context_entries.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    index = 0;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];
        s_log_context_view view;

        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        view.key = entry.key.c_str();
        view.value = entry.value.c_str();
        view.has_value = entry.has_value;
        snapshot.push_back(view);
        error_code = snapshot.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        index += 1;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void logger_context_clear()
{
    size_t count;
    int error_code;

    count = g_log_context_entries.size();
    error_code = g_log_context_entries.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    while (count > 0)
    {
        g_log_context_entries.pop_back();
        error_code = g_log_context_entries.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return ;
        }
        count -= 1;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_log_context_push(const s_log_field *fields, size_t field_count)
{
    size_t pushed_count;

    if (logger_context_push(fields, field_count, &pushed_count) != 0)
        return (-1);
    return (0);
}

void ft_log_context_pop(size_t field_count)
{
    logger_context_pop(field_count);
    return ;
}

void ft_log_context_clear()
{
    logger_context_clear();
    return ;
}

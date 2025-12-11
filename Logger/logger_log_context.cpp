#include "logger_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static thread_local ft_vector<s_log_context_entry> g_log_context_entries;

static void logger_context_rollback(size_t count)
{
    size_t index;

    index = 0;
    while (index < count)
    {
        g_log_context_entries.pop_back();
        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
            return ;
        index += 1;
    }
    return ;
}

int logger_context_push(const s_log_field *fields, size_t field_count,
        size_t *pushed_count)
{
    size_t index;

    if (!pushed_count)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *pushed_count = 0;
    if (field_count == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (!fields)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    index = 0;
    while (index < field_count)
    {
        const s_log_field *field;
        s_log_context_entry entry;
        bool lock_acquired;

        field = &fields[index];
        lock_acquired = false;
        if (log_field_lock(field, &lock_acquired) != 0)
        {
            logger_context_rollback(index);
            return (-1);
        }
        if (!field->key)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(index);
            return (-1);
        }
        entry.key = field->key;
        if (entry.key.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = entry.key.get_error();
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(index);
            return (-1);
        }
        if (field->value)
        {
            entry.value = field->value;
            if (entry.value.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = entry.value.get_error();
                log_field_unlock(field, lock_acquired);
                logger_context_rollback(index);
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
        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            logger_context_rollback(index);
            return (-1);
        }
        index += 1;
    }
    *pushed_count = field_count;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void logger_context_pop(size_t count)
{
    size_t available;

    available = g_log_context_entries.size();
    if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_log_context_entries.get_error();
        return ;
    }
    if (count > available)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        count = available;
    }
    else
        ft_errno = FT_ERR_SUCCESSS;
    while (count > 0)
    {
        g_log_context_entries.pop_back();
        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            return ;
        }
        count -= 1;
    }
    return ;
}

static int logger_context_format_prefix(ft_string &prefix)
{
    size_t count;
    size_t index;
    bool first_entry;

    count = g_log_context_entries.size();
    if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_log_context_entries.get_error();
        return (-1);
    }
    if (count == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    prefix.append('[');
    if (prefix.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = prefix.get_error();
        return (-1);
    }
    index = 0;
    first_entry = true;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];

        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            return (-1);
        }
        if (!first_entry)
        {
            prefix.append(' ');
            if (prefix.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = prefix.get_error();
                return (-1);
            }
        }
        else
            first_entry = false;
        prefix.append(entry.key);
        if (prefix.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = prefix.get_error();
            return (-1);
        }
        if (entry.has_value)
        {
            prefix.append('=');
            if (prefix.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = prefix.get_error();
                return (-1);
            }
            prefix.append(entry.value);
            if (prefix.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = prefix.get_error();
                return (-1);
            }
        }
        index += 1;
    }
    prefix.append(']');
    if (prefix.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = prefix.get_error();
        return (-1);
    }
    prefix.append(' ');
    if (prefix.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = prefix.get_error();
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
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
            ft_errno = FT_ERR_SUCCESSS;
        return (result);
    }
    if (prefix.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = prefix.get_error();
        return (-1);
    }
    ft_string combined(prefix);
    if (combined.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = combined.get_error();
        return (-1);
    }
    combined.append(text);
    if (combined.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = combined.get_error();
        return (-1);
    }
    text = combined;
    if (text.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = text.get_error();
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
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

    if (!value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!logger_context_value_needs_quotes(value))
    {
        output.append(value);
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    output.append('"');
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
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
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
                return (-1);
            }
            output.append(character);
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
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
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
                return (-1);
            }
        }
        else
        {
            output.append(character);
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
                return (-1);
            }
        }
        index += 1;
    }
    output.append('"');
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

int logger_context_format_flat(ft_string &output)
{
    size_t count;
    size_t index;
    bool first_entry;

    output.clear();
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (-1);
    }
    count = g_log_context_entries.size();
    if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_log_context_entries.get_error();
        return (-1);
    }
    if (count == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    index = 0;
    first_entry = true;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];

        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            return (-1);
        }
        if (!first_entry)
        {
            output.append(' ');
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
                return (-1);
            }
        }
        else
            first_entry = false;
        output.append(entry.key);
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = output.get_error();
            return (-1);
        }
        if (entry.has_value)
        {
            output.append('=');
            if (output.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = output.get_error();
                return (-1);
            }
            if (entry.value.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = entry.value.get_error();
                return (-1);
            }
            if (logger_context_append_flat_value(output, entry.value.c_str()) != 0)
                return (-1);
        }
        index += 1;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

int logger_context_snapshot(ft_vector<s_log_context_view> &snapshot)
{
    size_t count;
    size_t index;

    snapshot.clear();
    if (snapshot.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = snapshot.get_error();
        return (-1);
    }
    count = g_log_context_entries.size();
    if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_log_context_entries.get_error();
        return (-1);
    }
    index = 0;
    while (index < count)
    {
        const s_log_context_entry &entry = g_log_context_entries[index];
        s_log_context_view view;

        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            return (-1);
        }
        view.key = entry.key.c_str();
        view.value = entry.value.c_str();
        view.has_value = entry.has_value;
        snapshot.push_back(view);
        if (snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = snapshot.get_error();
            return (-1);
        }
        index += 1;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void logger_context_clear()
{
    size_t count;

    count = g_log_context_entries.size();
    if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_log_context_entries.get_error();
        return ;
    }
    while (count > 0)
    {
        g_log_context_entries.pop_back();
        if (g_log_context_entries.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_log_context_entries.get_error();
            return ;
        }
        count -= 1;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int ft_log_context_push(const s_log_field *fields, size_t field_count)
{
    size_t pushed_count;

    if (logger_context_push(fields, field_count, &pushed_count) != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
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


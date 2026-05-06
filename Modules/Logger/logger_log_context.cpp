#include "logger_internal.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

static thread_local ft_vector<s_log_context_entry> g_log_context_entries;

static int32_t logger_context_ensure_entries_ready()
{
    uint8_t state_value;
    int32_t initialize_error;

    state_value = g_log_context_entries.is_initialised();
    if (state_value == FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    initialize_error = g_log_context_entries.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    return (FT_ERR_SUCCESS);
}

static int32_t logger_context_rollback(ft_size_t entry_count)
{
    ft_size_t entry_index;

    entry_index = 0;
    while (entry_index < entry_count)
    {
        g_log_context_entries.pop_back();
        int32_t vector_error = g_log_context_entries.get_error();
        if (vector_error != FT_ERR_SUCCESS)
            return (vector_error);
        entry_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int32_t logger_context_push(const s_log_field *fields, ft_size_t field_count,
        ft_size_t *pushed_count)
{
    ft_size_t entry_index;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);
    if (!pushed_count)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    *pushed_count = 0;
    if (field_count == 0)
    {
        return (FT_ERR_SUCCESS);
    }
    if (!fields)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry_index = 0;
    while (entry_index < field_count)
    {
        const s_log_field *field;
        s_log_context_entry entry;
        ft_bool lock_acquired;
        int32_t error_code_value;

        field = &fields[entry_index];
        lock_acquired = FT_FALSE;
        error_code_value = log_field_lock(field, &lock_acquired);
        if (error_code_value != FT_ERR_SUCCESS)
        {
            logger_context_rollback(entry_index);
            return (error_code_value);
        }
        if (!field->key)
        {
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(entry_index);
            return (FT_ERR_INVALID_ARGUMENT);
        }
        error_code_value = entry.key.initialize(field->key);
        if (error_code_value != FT_ERR_SUCCESS)
        {
            log_field_unlock(field, lock_acquired);
            logger_context_rollback(entry_index);
            return (error_code_value);
        }
        if (field->value)
        {
            error_code_value = entry.value.initialize(field->value);
            if (error_code_value != FT_ERR_SUCCESS)
            {
                log_field_unlock(field, lock_acquired);
                logger_context_rollback(entry_index);
                return (error_code_value);
            }
            entry.has_value = FT_TRUE;
        }
        else
        {
            error_code_value = entry.value.initialize();
            if (error_code_value != FT_ERR_SUCCESS)
            {
                log_field_unlock(field, lock_acquired);
                logger_context_rollback(entry_index);
                return (error_code_value);
            }
            entry.has_value = FT_FALSE;
        }
        log_field_unlock(field, lock_acquired);
        g_log_context_entries.push_back(entry);
        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            logger_context_rollback(entry_index);
            return (error_code_value);
        }
        entry_index += 1;
    }
    *pushed_count = field_count;
    return (FT_ERR_SUCCESS);
}

void logger_context_pop(ft_size_t entry_count)
{
    ft_size_t available;
    int32_t error_code_value;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return ;
    available = g_log_context_entries.size();
    error_code_value = g_log_context_entries.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (entry_count > available)
    {
        error_code_value = FT_ERR_INVALID_OPERATION;
        entry_count = available;
    }
    else
        error_code_value = FT_ERR_SUCCESS;
    while (entry_count > 0)
    {
        g_log_context_entries.pop_back();
        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return ;
        }
        entry_count -= 1;
    }
    return ;
}

static int32_t logger_context_format_prefix(ft_string &prefix)
{
    ft_size_t entry_count;
    ft_size_t entry_index;
    ft_bool first_entry;
    int32_t error_code_value;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);
    if (prefix.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    entry_count = g_log_context_entries.size();
    error_code_value = g_log_context_entries.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    if (entry_count == 0)
    {
        return (FT_ERR_SUCCESS);
    }
    error_code_value = prefix.append('[');
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    entry_index = 0;
    first_entry = FT_TRUE;
    while (entry_index < entry_count)
    {
        const s_log_context_entry &entry = g_log_context_entries[entry_index];

        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        if (!first_entry)
        {
            error_code_value = prefix.append(' ');
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
        }
        else
            first_entry = FT_FALSE;
        error_code_value = prefix.append(entry.key);
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        if (entry.has_value)
        {
            error_code_value = prefix.append('=');
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
            error_code_value = prefix.append(entry.value);
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
        }
        entry_index += 1;
    }
    error_code_value = prefix.append(']');
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    error_code_value = prefix.append(' ');
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    return (1);
}

int32_t logger_context_apply_plain(ft_string &text)
{
    ft_string prefix;
    int32_t operation_result;

    operation_result = logger_context_format_prefix(prefix);
    if (operation_result < 0)
    {
        return (operation_result);
    }
    if (operation_result == 0)
    {
        return (operation_result);
    }
    ft_string combined;
    int32_t error_code_value = combined.initialize(prefix);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    error_code_value = combined.append(text);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    error_code_value = text.initialize(combined);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    return (FT_ERR_SUCCESS);
}

static ft_bool logger_context_value_needs_quotes(const char *value)
{
    ft_size_t entry_index;

    if (!value)
        return (FT_FALSE);
    entry_index = 0;
    while (value[entry_index] != '\0')
    {
        unsigned char character;

        character = static_cast<unsigned char>(value[entry_index]);
        if (character <= ' ' || character == '"' || character == '\\' || character == '=')
            return (FT_TRUE);
        entry_index += 1;
    }
    return (FT_FALSE);
}

static int32_t logger_context_append_flat_value(ft_string &output, const char *value)
{
    ft_size_t entry_index;
    static const char hex_digits[] = "0123456789ABCDEF";
    int32_t error_code_value;

    if (!value)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (!logger_context_value_needs_quotes(value))
    {
        error_code_value = output.append(value);
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        return (FT_ERR_SUCCESS);
    }
    error_code_value = output.append('"');
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    entry_index = 0;
    while (value[entry_index] != '\0')
    {
        char character;

        character = value[entry_index];
        if (character == '"' || character == '\\')
        {
            error_code_value = output.append('\\');
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
            error_code_value = output.append(character);
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
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
            error_code_value = output.append(escape_buffer);
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
        }
        else
        {
            error_code_value = output.append(character);
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
        }
        entry_index += 1;
    }
    error_code_value = output.append('"');
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    return (FT_ERR_SUCCESS);
}

int32_t logger_context_format_flat(ft_string &output)
{
    ft_size_t entry_count;
    ft_size_t entry_index;
    ft_bool first_entry;
    int32_t error_code_value;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);
    error_code_value = output.clear();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    entry_count = g_log_context_entries.size();
    error_code_value = g_log_context_entries.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    if (entry_count == 0)
    {
        return (FT_ERR_SUCCESS);
    }
    entry_index = 0;
    first_entry = FT_TRUE;
    while (entry_index < entry_count)
    {
        const s_log_context_entry &entry = g_log_context_entries[entry_index];

        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        if (!first_entry)
        {
            error_code_value = output.append(' ');
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
        }
        else
            first_entry = FT_FALSE;
        error_code_value = output.append(entry.key);
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        if (entry.has_value)
        {
            error_code_value = output.append('=');
            if (error_code_value != FT_ERR_SUCCESS)
            {
                return (error_code_value);
            }
            if (logger_context_append_flat_value(output, entry.value.c_str()) != 0)
                return (FT_ERR_INVALID_ARGUMENT);
        }
        entry_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int32_t logger_context_snapshot(ft_vector<s_log_context_view> &snapshot)
{
    ft_size_t entry_count;
    ft_size_t entry_index;
    int32_t error_code_value;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);
    if (snapshot.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    snapshot.clear();
    error_code_value = snapshot.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    entry_count = g_log_context_entries.size();
    error_code_value = g_log_context_entries.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return (error_code_value);
    }
    entry_index = 0;
    while (entry_index < entry_count)
    {
        const s_log_context_entry &entry = g_log_context_entries[entry_index];
        s_log_context_view view;

        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        view.key = entry.key.c_str();
        if (entry.has_value)
            view.value = entry.value.c_str();
        else
            view.value = ft_nullptr;
        view.has_value = entry.has_value;
        snapshot.push_back(view);
        error_code_value = snapshot.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return (error_code_value);
        }
        entry_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

void logger_context_clear()
{
    ft_size_t entry_count;
    int32_t error_code_value;
    int32_t ensure_error;

    ensure_error = logger_context_ensure_entries_ready();
    if (ensure_error != FT_ERR_SUCCESS)
        return ;
    entry_count = g_log_context_entries.size();
    error_code_value = g_log_context_entries.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        return ;
    }
    while (entry_count > 0)
    {
        g_log_context_entries.pop_back();
        error_code_value = g_log_context_entries.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
        {
            return ;
        }
        entry_count -= 1;
    }
    return ;
}

int32_t ft_log_context_push(const s_log_field *fields, ft_size_t field_count)
{
    ft_size_t pushed_count;

    if (logger_context_push(fields, field_count, &pushed_count) != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_SUCCESS);
}

void ft_log_context_pop(ft_size_t field_count)
{
    logger_context_pop(field_count);
    return ;
}

void ft_log_context_clear()
{
    logger_context_clear();
    return ;
}

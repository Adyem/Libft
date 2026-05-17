#include "logger_internal.hpp"
#include "../Basic/basic.hpp"

ft_vector<s_redaction_rule> g_redaction_rules;

int32_t s_redaction_rule::initialize(const s_redaction_rule &other) noexcept
{
    int32_t error_code;

    error_code = this->pattern.initialize(other.pattern);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->replacement.initialize(other.replacement);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int32_t s_redaction_rule::destroy() noexcept
{
    int32_t first_error;
    int32_t error_code;

    first_error = this->pattern.destroy();
    error_code = this->replacement.destroy();
    if (first_error == FT_ERR_SUCCESS && error_code != FT_ERR_SUCCESS)
        first_error = error_code;
    return (first_error);
}

static int32_t logger_apply_redaction_rule(ft_string &text,
    const s_redaction_rule &rule)
{
    const char *pattern;
    const char *replacement;
    ft_size_t pattern_length;
    ft_size_t text_length;
    ft_size_t entry_index;
    const char *source;
    ft_string operation_result;
    int32_t error_code_value;

    if (operation_result.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    if (!text.is_initialised() || !rule.pattern.is_initialised()
        || !rule.replacement.is_initialised())
        return (FT_ERR_NOT_INITIALISED);
    pattern = rule.pattern.c_str();
    pattern_length = rule.pattern.size();
    if (pattern_length == 0)
        return (FT_ERR_SUCCESS);
    replacement = rule.replacement.c_str();
    text_length = text.size();
    source = text.c_str();
    if (!source)
        source = "";
    entry_index = 0;
    while (entry_index < text_length)
    {
        ft_size_t match_index;

        match_index = 0;
        while (entry_index + match_index < text_length
            && match_index < pattern_length
            && source[entry_index + match_index] == pattern[match_index])
            match_index++;
        if (match_index == pattern_length)
        {
            error_code_value = operation_result.append(replacement);
            if (error_code_value != FT_ERR_SUCCESS)
                return (error_code_value);
            entry_index += pattern_length;
        }
        else
        {
            error_code_value = operation_result.append(source[entry_index]);
            if (error_code_value != FT_ERR_SUCCESS)
                return (error_code_value);
            entry_index++;
        }
    }
    error_code_value = text.destroy();
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    error_code_value = text.initialize(operation_result);
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_add_redaction_with_replacement(const char *pattern,
    const char *replacement)
{
    const char  *replacement_value;
    int32_t     unlock_status;
    int32_t     error_code_value;
    int32_t     lock_error;
    s_redaction_rule rule;

    if (!pattern || !pattern[0])
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    replacement_value = replacement;
    if (!replacement_value)
        replacement_value = "[REDACTED]";
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    error_code_value = rule.pattern.initialize(pattern);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        unlock_status = logger_unlock_sinks();
        if (unlock_status != FT_ERR_SUCCESS)
            return (unlock_status);
        return (error_code_value);
    }
    error_code_value = rule.replacement.initialize(replacement_value);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        unlock_status = logger_unlock_sinks();
        if (unlock_status != FT_ERR_SUCCESS)
            return (unlock_status);
        return (error_code_value);
    }
    g_redaction_rules.push_back(rule);
    error_code_value = g_redaction_rules.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
    {
        unlock_status = logger_unlock_sinks();
        if (unlock_status != FT_ERR_SUCCESS)
            return (unlock_status);
        return (error_code_value);
    }
    unlock_status = logger_unlock_sinks();
    if (unlock_status != FT_ERR_SUCCESS)
    {
        return (unlock_status);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_add_redaction(const char *pattern)
{
    return (ft_log_add_redaction_with_replacement(pattern, "[REDACTED]"));
}

void ft_log_clear_redactions()
{
    int32_t unlock_status;
    int32_t clear_error;
    int32_t lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    g_redaction_rules.clear();
    clear_error = g_redaction_rules.get_error();
    unlock_status = logger_unlock_sinks();
    if (unlock_status != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (clear_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    return ;
}

int32_t logger_copy_redaction_rules(ft_vector<s_redaction_rule> &destination)
{
    ft_size_t rule_count;
    ft_size_t entry_index;
    int32_t error_code_value;
    uint8_t destination_state;
    int32_t destination_initialize_error;

    destination_state = destination.is_initialised();
    if (destination_state != FT_CLASS_STATE_INITIALISED)
    {
        destination_initialize_error = destination.initialize();
        if (destination_initialize_error != FT_ERR_SUCCESS)
            return (destination_initialize_error);
    }
    rule_count = g_redaction_rules.size();
    error_code_value = g_redaction_rules.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    entry_index = 0;
    while (entry_index < rule_count)
    {
        const s_redaction_rule &source_rule = g_redaction_rules[entry_index];
        error_code_value = g_redaction_rules.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        destination.push_back(source_rule);
        error_code_value = destination.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        entry_index++;
    }
    error_code_value = FT_ERR_SUCCESS;
    return (error_code_value);
}

int32_t logger_apply_redactions(ft_string &text,
    const ft_vector<s_redaction_rule> &rules)
{
    ft_size_t rule_count;
    ft_size_t entry_index;
    int32_t error_code_value;

    if (!rules.is_initialised())
        return (FT_ERR_NOT_INITIALISED);
    if (!text.is_initialised())
        return (FT_ERR_NOT_INITIALISED);
    rule_count = rules.size();
    error_code_value = rules.get_error();
    if (error_code_value != FT_ERR_SUCCESS)
        return (error_code_value);
    entry_index = 0;
    while (entry_index < rule_count)
    {
        int32_t             apply_error;

        const s_redaction_rule &source_rule = rules[entry_index];
        error_code_value = rules.get_error();
        if (error_code_value != FT_ERR_SUCCESS)
            return (error_code_value);
        apply_error = logger_apply_redaction_rule(text, source_rule);
        if (apply_error != FT_ERR_SUCCESS)
            return (apply_error);
        entry_index++;
    }
    error_code_value = FT_ERR_SUCCESS;
    return (error_code_value);
}

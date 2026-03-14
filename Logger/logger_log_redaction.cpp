#include "logger_internal.hpp"
#include "../Basic/basic.hpp"

ft_vector<s_redaction_rule> g_redaction_rules;

static int32_t logger_apply_redaction_rule(ft_string &text,
    const s_redaction_rule &rule)
{
    const char  *pattern;
    const char  *replacement;
    ft_size_t      pattern_length;
    ft_size_t      text_length;
    ft_size_t      entry_index;
    const char  *source;
    ft_string   operation_result;

    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    pattern = rule.pattern.c_str();
    if (!pattern)
        return (FT_ERR_SUCCESS);
    pattern_length = rule.pattern.size();
    if (pattern_length == 0)
        return (FT_ERR_SUCCESS);
    replacement = rule.replacement.c_str();
    text_length = text.size();
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
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
            operation_result.append(replacement);
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (ft_string::get_error());
            entry_index += pattern_length;
            continue ;
        }
        operation_result.push_back(source[entry_index]);
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (ft_string::get_error());
        entry_index++;
    }
    text = operation_result;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_add_redaction_with_replacement(const char *pattern,
    const char *replacement)
{
    const char  *replacement_value;
    int32_t         unlock_status;
    int32_t         error_code_value;
    int32_t         lock_error;
    s_redaction_rule rule;

    if (!pattern || !pattern[0])
    {
        return (FT_ERR_INTERNAL);
    }
    replacement_value = replacement;
    if (!replacement_value)
        replacement_value = "[REDACTED]";
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (FT_ERR_INTERNAL);
    }
    rule.pattern = pattern;
    rule.replacement = replacement_value;
    if (ft_string::get_error() != FT_ERR_SUCCESS
        || ft_string::get_error() != FT_ERR_SUCCESS)
    {
        error_code_value = ft_string::get_error();
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = ft_string::get_error();
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_NO_MEMORY;
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return (FT_ERR_INTERNAL);
        }
        return (FT_ERR_INTERNAL);
    }
    g_redaction_rules.push_back(rule);
    if (g_redaction_rules.get_error() != FT_ERR_SUCCESS)
    {
        error_code_value = g_redaction_rules.get_error();
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return (FT_ERR_INTERNAL);
        }
        return (FT_ERR_INTERNAL);
    }
    unlock_status = logger_unlock_sinks();
    if (unlock_status != FT_ERR_SUCCESS)
    {
        return (FT_ERR_INTERNAL);
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

    rule_count = g_redaction_rules.size();
    if (g_redaction_rules.get_error() != FT_ERR_SUCCESS)
        return (g_redaction_rules.get_error());
    entry_index = 0;
    while (entry_index < rule_count)
    {
        s_redaction_rule rule;

        rule = g_redaction_rules[entry_index];
        if (g_redaction_rules.get_error() != FT_ERR_SUCCESS)
            return (g_redaction_rules.get_error());
        destination.push_back(rule);
        if (destination.get_error() != FT_ERR_SUCCESS)
            return (destination.get_error());
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

    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    rule_count = rules.size();
    if (rules.get_error() != FT_ERR_SUCCESS)
        return (rules.get_error());
    entry_index = 0;
    while (entry_index < rule_count)
    {
        s_redaction_rule rule;
        int32_t             apply_error;

        rule = rules[entry_index];
        if (rules.get_error() != FT_ERR_SUCCESS)
            return (rules.get_error());
        apply_error = logger_apply_redaction_rule(text, rule);
        if (apply_error != FT_ERR_SUCCESS)
            return (apply_error);
        entry_index++;
    }
    error_code_value = FT_ERR_SUCCESS;
    return (error_code_value);
}
